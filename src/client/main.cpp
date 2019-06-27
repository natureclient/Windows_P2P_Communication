#include <Windows.h>
#include <stdio.h>

#define BUFSIZE 1024

int initWinsock(WORD wVersionRequested, LPWSADATA lpWSAData, HANDLE text);
int createSocket(SOCKET *sock, int af, int type, int protocol, HANDLE text);
int connectToHost(SOCKET sock, struct sockaddr_in *addr, int addrlen, HANDLE text);
void fillCLTAddr(struct sockaddr_in *addr, char ip[16], int port);
void sendMSG(SOCKET sock);
DWORD WINAPI recvData(LPVOID sock);
void EXIT(HANDLE text);

/* ================================================================================================================== */
int main(int argc, char **argv)
{
	WSADATA wsaData; // struct containing returning info about DLL
	WORD wVersionRequested = MAKEWORD(2, 2); // version 2.2 of winsock
	SOCKET cltSock;
	struct sockaddr_in cltAddr;
	DWORD threadId;
	HANDLE recvThread;
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);  // Get handle to standard output
	int portArg;

	// set color of text
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("=====================================================\n");

	// check for port via CMD ARGS 
	if (argc < 3)
	{
		SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED);
		printf("Missing IP and Port CMD ARG\n\n\tUsage: <executable> <IP> <PORT>\n\n Shutting down.\n");
		EXIT(hConsole);
	}

	// get port specified via CMD ARGS
	portArg = atoi(argv[2]);

	// initialize winsock DLL wrapper function
	if (initWinsock(wVersionRequested, &wsaData, hConsole) != 0)
		EXIT(hConsole);

	// create socket
	if (createSocket(&cltSock, AF_INET, SOCK_STREAM, 0, hConsole) != 0)
		EXIT(hConsole);

	// fill server sockaddr struct
	memset(&cltAddr, 0, sizeof(cltAddr));
	fillCLTAddr(&cltAddr, argv[1], portArg);
	
	// connect to server
	if (connectToHost(cltSock, &cltAddr, sizeof(cltAddr), hConsole) != 0)
		EXIT(hConsole);

	// formatting
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN);
	printf("\t> Accepted/Connected Sock: %i\n", cltSock);
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("=====================================================\n");
	SetConsoleTextAttribute(hConsole, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	printf("\t\tCommunication Enabled\n");

	// create new thread to perfrom read operations on socket
	recvThread = CreateThread(NULL, 0, recvData, (void*)cltSock, 0, &threadId);
	
	// send message loop
	sendMSG(cltSock);

	system("pause");

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
		printf("\t>  creating socket\n\t\tError code: %i\n", WSAGetLastError());
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
|	FUNCTION:		int connectToHost(SOCKET sock, struct sockaddr_in *addr, int addrlen)
|						SOCKET sock				 : socket to connect with
|						struct sockaddr_in *addr : socket address struct to use for connection
|						int addrlen				 : size of the clients sockaddr_in struct
|
|	RETURN:			0 on success, SOCKET_ERROR on failure
|
|	DESCRIPTION:
|		Wrapper function to connect to host
------------------------------------------------------------------------------------*/
int connectToHost(SOCKET sock, struct sockaddr_in *addr, int addrlen, HANDLE text)
{
	// hold result of connect()
	int res;

	// connect to server
	res = connect(sock, (struct sockaddr *)addr, addrlen);

	// error check
	if (res != 0)
	{
		SetConsoleTextAttribute(text, FOREGROUND_INTENSITY | FOREGROUND_RED);
		printf("\t> Error connecting to host\n\t\tError code: %i\n", WSAGetLastError());
	}
	else
	{
		printf("\t> Successfully connected to server\n");
	}

	return res;
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
void fillCLTAddr(struct sockaddr_in *addr, char ip[16], int port)
{
	addr->sin_family = AF_INET; // protocol TCP/IP
	addr->sin_port = htons(port); // port
	addr->sin_addr.s_addr = inet_addr(ip); // IP to accept (any)
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
			printf("\n\nPEER:\n%s\n", incomingDataBuffer);
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
	
	printf("\nExiting....\n");
	WSACleanup();
	system("pause");
	exit(1);
}