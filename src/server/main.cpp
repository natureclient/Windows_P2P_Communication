#pragma once
#include <Windows.h>
#include <stdio.h>

#define BACKLOG 5
#define BUFSIZE 1024

int initWinsock(WORD wVersionRequested, LPWSADATA lpWSAData, HANDLE text);
int createSocket(SOCKET *sock, int af, int type, int protocol, HANDLE text);
void fillServAddrStruct(struct sockaddr_in *addr, int port);
int bindSocket(SOCKET sock, struct sockaddr_in *addr, int addrLen, HANDLE text);
int setToListen(SOCKET sock, int backlog, HANDLE text);
int acceptConnection(SOCKET *accSock, SOCKET lisSock, struct sockaddr_in *cltAddr, int *cltAddrLen, HANDLE text);
void sendMSG(SOCKET sock);
DWORD WINAPI recvData(LPVOID sock);
void EXIT(HANDLE text);

/* ================================================================================================================== */
int main(int argc, char **argv)
{
	SOCKET listenSock; // servers socket
	SOCKET acceptSock; // client socket
	WSADATA WSImp;
	WORD WSVersion = MAKEWORD(2, 2);
	DWORD threadId;
	HANDLE recvThread;
	struct sockaddr_in servAddr;
	struct sockaddr_in cltAddr;
	int cltAddrLen;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
	int portArg;

	// set color of text
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("=====================================================\n");

	// check for port via CMD ARGS 
	if (argc < 2)
	{	
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
		printf("Missing Port CMD ARG. Shutting down.\n");
		EXIT(hConsole);
	}

	// get port specified via CMD ARGS
	portArg = atoi(argv[1]);

	// initialize winsock DLL wrapper function
	if (initWinsock(WSVersion, &WSImp, hConsole) != 0)
		EXIT(hConsole);
	
	// create socket
	if (createSocket(&listenSock, AF_INET, SOCK_STREAM, 0, hConsole) != 0)
		EXIT(hConsole);

	// fill server sockaddr struct
	memset((char *)&servAddr, 0, sizeof(struct sockaddr_in));
	fillServAddrStruct(&servAddr, portArg);

	// bind socket to port
	if (bindSocket(listenSock, &servAddr, sizeof(servAddr), hConsole) != 0)
		EXIT(hConsole);

	// set socket to listen
	if (setToListen(listenSock, BACKLOG, hConsole) != 0)
		EXIT(hConsole);

	// check socket descriptor
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	printf("\t> Listen Sock: %i\n", listenSock);
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);

	// accept any incoming connection
	cltAddrLen = sizeof(cltAddr);
	if (acceptConnection(&acceptSock, listenSock, &cltAddr, &cltAddrLen, hConsole) != 0)
		EXIT(hConsole);

	// formatting
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	printf("\t> Accepted/Connected Sock: %i\n", acceptSock);
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("=====================================================\n");
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("\t\tCommunication Enabled\n");

	// create new thread to perfrom read operations on socket
	recvThread = CreateThread(NULL, 0, recvData, (void*)acceptSock, 0, &threadId);
	
	// send message loop
	sendMSG(acceptSock);

	system("pause");

	// close thread
	CloseHandle(recvThread);

	// clean up before shutting down
	WSACleanup();
	return 0;
}
/* ================================================================================================================== */


/*------------------------------------------------------------------------------------
|	FUNCTION:		int initWinsock(WORD wVersionRequested, LPWSADATA lpWSAData)
|						WORD wVersionRequested	:	version of winsock to use
|						LPWSADATA lpWSAData		:	WSADATA struct for config info
|
|	RETURN:			0 on success, error code on error
|
|	DESCRIPTION:
|		Wrapper function for WSAStartup. Contains a result variable because
|		WSAGetLastError should not be used (according to msdn docs)
------------------------------------------------------------------------------------*/
int initWinsock(WORD wVersionRequested, LPWSADATA lpWSAData, HANDLE text)
{
	// holds result of WSAStartup
	int res;

	// initialize the winsock DLL
	res = WSAStartup(wVersionRequested, lpWSAData);
	
	// error check
	if (res != 0)
	{
		SetConsoleTextAttribute(text, FOREGROUND_INTENSITY | FOREGROUND_RED);
		printf("\t> Error initializing winsock DLL\n\t\tError code: %i\n", res);
	}
	else
		printf("\t> Successfully initialized winsock\n");

	return res;
}


/*------------------------------------------------------------------------------------
|	FUNCTION:		int createSocket(SOCKET *sock, int af, int type, int protocol)
|						SOCKET *sock		:	pointer to socket to update
|						int af				:	address family (ipv4)
|						int type			:	type of connection (tcp/udp)
|						int protocol		:	protocol to use
|
|	RETURN:			0 on success, 1 on error
|
|	DESCRIPTION:
|		Wrapper function for to create socket
------------------------------------------------------------------------------------*/
int createSocket(SOCKET *sock, int af, int type, int protocol, HANDLE text)
{
	// temporarily holds to socket descriptor
	SOCKET tmpSock;

	// create socket
	tmpSock = socket(af, type, protocol);

	// error check
	if (tmpSock == INVALID_SOCKET)
	{
		SetConsoleTextAttribute(text, FOREGROUND_INTENSITY | FOREGROUND_RED);
		printf("\t> Error creating socket\n\t\tError code: %i\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("\t> Successfully created socket\n");
	}

	*sock = tmpSock; // assign sock descriptor to Socket

	return 0;
}


/*------------------------------------------------------------------------------------
|	FUNCTION:		void fillSockStruct(struct sockaddr_in *sockStruct)
|						struct sockaddr_in *sockStruct : socket address struct to fill out
|
|	RETURN:			void
|
|	DESCRIPTION:
|		Fills out an IPv4 socket address structure
------------------------------------------------------------------------------------*/
void fillServAddrStruct(struct sockaddr_in *addr, int port)
{
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port); // htons returns port value in TCP/IP network byte order (u_short)
	addr->sin_addr.s_addr = htonl(INADDR_ANY); // htonl returns ipv4 addres in TCP/IP network byte order (u_long)
}


/*------------------------------------------------------------------------------------
|	FUNCTION:		int bindSocket(SOCKET sock, struct sockaddr_in *serv, int addrLen)
|						SOCKET sock : socket to bind
|						struct sockaddr_in *serv : pointer to ipv4 address structure
|						int addrLen : length of address structure
|
|	RETURN:			return 0 on success, SOCKET_ERROR on error
|
|	DESCRIPTION:
|		Wrapper function to bind socket
------------------------------------------------------------------------------------*/
int bindSocket(SOCKET sock, struct sockaddr_in *addr, int addrLen, HANDLE text)
{
	int res;

	// bind socket to port
	res = bind(sock, (struct sockaddr *)addr, addrLen);

	// error check
	if (res != 0)
	{
		SetConsoleTextAttribute(text, FOREGROUND_INTENSITY | FOREGROUND_RED);
		printf("\t> Error binding socket\n\tError code: %i\n", WSAGetLastError());
	}
	else
		printf("\t> Successfully bound socket\n");

	return res;
}


/*------------------------------------------------------------------------------------
|	FUNCTION:		int setToListen(SOCKET sock, int backlog)
|						SOCKET sock : socket to listen for incoming connections on
|						int backlog : max num of clients waiting to connect in queue
|
|	RETURN:			return 0 on success, SOCKET_ERROR on error
|
|	DESCRIPTION:
|		Wrapper function to set socket to listening
------------------------------------------------------------------------------------*/
int setToListen(SOCKET sock, int backlog, HANDLE text)
{
	int res; // hold result of listen()

	// set socket to listen
	res = listen(sock, backlog);
	
	// error check
	if (res != 0)
	{
		SetConsoleTextAttribute(text, FOREGROUND_INTENSITY | FOREGROUND_RED);
		printf("\t> Error setting socket to listen\n\tError code: %i\n", res);
	}
	else
		printf("\t> Successfully set socket to listen\n");

	return res;
}


/*------------------------------------------------------------------------------------
|	FUNCTION:		int acceptConnection(SOCKET srvSock, SOCKET *cltSock, struct sockaddr_in *client, int *addrLen)
|						SOCKET srvSock : listening socket
|						SOCKET *cltSock : new socket of the accepted connection
|						struct sockaddr_in *client : pointer to address struct of new connection
|						int *addrLen : length of the struct pointed to by client (^^^)
|
|	RETURN:			return 0 on success, 1 on error
|
|	DESCRIPTION:
|		Accepts a connection when one is present.
------------------------------------------------------------------------------------*/
int acceptConnection(SOCKET *accSock, SOCKET lisSock, struct sockaddr_in *cltAddr, int *cltAddrLen, HANDLE text)
{
	// create new socket by accepting connection
	*accSock = accept(lisSock, (struct sockaddr *)cltAddr, cltAddrLen);

	// error check
	if (*accSock == INVALID_SOCKET)
	{
		SetConsoleTextAttribute(text, FOREGROUND_INTENSITY | FOREGROUND_RED);
		printf("\t> Error accepting connection\n\tError code: %i\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("\t> Successfully accepted socket\n");
	}

	return 0;
}


/*------------------------------------------------------------------------------------
|	FUNCTION:		DWORD WINAPI recvData(LPVOID sock)
|						LPVOID sock : connected socket to recieve messages thru
|
|	RETURN:			0 on success, 1 on failure
|
|	DESCRIPTION:
|		Wrapper function for recieving a message thru a socket
------------------------------------------------------------------------------------*/
DWORD WINAPI recvData(LPVOID sock)
{
	char incomingDataBuffer[BUFSIZE];

	int res;

	// cast to get socket
	SOCKET tmpSock = (SOCKET)sock;

	//printf("Socket num: %i\n\n", tmpSock);
	memset(incomingDataBuffer, '\0', BUFSIZE);

	printf("\n");

	while (1)
	{
		// read data from socket
		res = recv(tmpSock, incomingDataBuffer, BUFSIZE, NULL);

		// error check
		if (res > 0) // if data read
		{
			//printf("FRIEND:\n%s\n\nBytes read: %d\n\n", incomingDataBuffer, res);
			printf("\nPEER:\n%s\n", incomingDataBuffer);
		}
		else if (res == 0) // if other side dropped connection
		{
			printf("\t> Friend left conversation\n\n");
			return 1;
		}
		else // error
		{
			printf("\n\t> Error reading socket\n\t\tError code: %i\n\n", WSAGetLastError());
			return 1;
		}
		memset(incomingDataBuffer, '\0', BUFSIZE);
	}

	return 0;
}


/*------------------------------------------------------------------------------------
|	FUNCTION:		void sendMSG(SOCKET sock)
|						SOCKET sock : connected socket to send messages thru
|
|	RETURN:			void
|
|	DESCRIPTION:
|		Wrapper function for sending a message thru a socket
------------------------------------------------------------------------------------*/
void sendMSG(SOCKET sock)
{
	char msgBuffer[BUFSIZE];
	int res;

	memset(msgBuffer, '\0', BUFSIZE);

	// send loop
	while (1)
	{
		fgets(msgBuffer, BUFSIZE, stdin); // get user input

		if (msgBuffer[0] == 'x') // if user specifies to exit
		{
			return;
		}
		else if (msgBuffer[0] != '\n') // user does not hit enter with an empty buffer
		{
			// send message thru socket
			res = send(sock, msgBuffer, BUFSIZE, NULL);

			// error check
			if (res == SOCKET_ERROR)
			{
				printf("\n\t> Error sending packet\n\t\tERROR CODE: %i\n\n", WSAGetLastError());
				return;
			}
		}

		memset(msgBuffer, '\0', BUFSIZE);
	}
}


/*------------------------------------------------------------------------------------
|	FUNCTION:		void EXIT()
|
|	RETURN:			void
|
|	DESCRIPTION:
|		Exits program
------------------------------------------------------------------------------------*/
void EXIT(HANDLE text)
{
	// formatting
	SetConsoleTextAttribute(text, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("=====================================================\n");
	SetConsoleTextAttribute(text, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);

	printf("\n\nExiting....\n");
	WSACleanup();
	system("pause");
	exit(1);
}