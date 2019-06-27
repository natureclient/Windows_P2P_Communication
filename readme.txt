==================================P2P COMM==================================
|								           |
|			 Peer to Peer Communication			   |
|			============================			   |
| Simple peer to peer console based messaging application for Windows.     |
| Using WinSock, two peers may connect to eachother and start sending	   |
| messages back and fourth.				   		   |
|									   |
| Creator: Alex Zielinski						   |						   
|									   |								   
| Guide									   |
| -----								  	   |
|									   |
|	- Server must be started first 					   |
|	  								   |
|	- client must be started second and connects to server		   |
|									   |
|									   |
|	- Server needs ONE command line argument. This CMD ARG defines the |
|	  port								   |
|									   |
|	- Client needs TWO CMD ARGS. 					   |
|		> 1st CMD ARG: defines the IP to connect to		   |
|		> 2nd CMD ARG: defines the port to use			   |
|									   |
|	  NOTE: the order of CMD ARGS when starting the client matter. As  |
|		mentioned earlier, first the IP and then the Port, as 	   |
|		follows:						   |
|									   |
|			   <executable> <ip> <port>			   |
|									   |
|	- Once the server and client have been started (assuming that the  |
|	  CMD ARGS provided are valid to create a connection) a 	   |
|	  connection will have been established and communication between  |
|	  the two peers may begin					   |
|									   |
|	- To EXIT the program (during communication) simply enter the 	   |
|	  char 'x' and press 'enter' and follow the on screen directions   |
|									   |
|	- To EXIT server when it is listening for clients simplt enter the |
|	  following keystrokes:						   |
|									   |
|			    	   ctrl + c				   |
|									   |
|===========================================================================