#include <stdio.h> // printf 
#include <stdlib.h> // for standard utilities
#include <string.h> // str functions
#include <sys/socket.h> // socket functions
#include <string> // use of strings
#include <arpa/inet.h> // for htons and inet_addr
#include <unistd.h> // for close()
#include <netdb.h> // hostent struct
#include <vector> // vectors
#include <thread> // threading controller
#include <pthread.h> // thread functions

// NOTES
// server socket on port 8000.
// controller socket is on port 8001.
// Compile with g++ proxy2.cpp -lpthread.
// enter command 'telnet localhost 8001' on seperate terminal after inital launch,
// before trying to connect with client.  controller needs to connect first.

using namespace std;

// vector of strings holding the list of blocked words
// updated during run, initialized with "Floppy"
std::vector<std::string> blockList {"Floppy"};

// Parses a HTTP request, returns a string of the path
string parsePath(const char* HTTP_req){      
    // find the start of the path
    const char *start_of_path = strchr(HTTP_req, ' ') + 1;
    // find the end of the path
    const char *end_of_path = strchr(start_of_path, ' ');
    // set char array length
    char path[end_of_path - start_of_path];
    // load the char array with the path
    strncpy(path, start_of_path, end_of_path - start_of_path);
    // terminate the path string
    path[sizeof(path)] = 0;
    printf("path/url is %s\n\n", path);
	return path;
}

// Parses an HTTP request, returns a string of the host field
string parseHost(const char* HTTP_req){
    // find the start of the host
	const char *start_of_host = strstr(HTTP_req, "\r\nHost: ") + 8;
    // find the end of the host
    const char *end_of_host = strchr(start_of_host, '\r');
    // set char array length
    char host[end_of_host - start_of_host];
    // load the char array with the host
    strncpy(host, start_of_host, end_of_host - start_of_host);
    // terminate the host string
    host[sizeof(host)] = 0;
    printf("host is %s\n\n", host);
    return host;
}

// Parses the controller message, returns the word to be blocked/unblocked
// controller must put a # at the end of the word 
string parseBlock(const char* block_req){
    // find the start of the word
	const char *start_of_block = strchr(block_req, ' ') + 1;
    // find the end of the word
    const char *end_of_block = strchr(start_of_block, '#');
    // set the char array length
    char block[end_of_block - start_of_block];
    // load the char array with the word
    strncpy(block, start_of_block, end_of_block - start_of_block);
    // terminate the word string
    block[sizeof(block)] = 0;
    return block;
}

// function for handling the controller inputs
// recieves messages on the given socket, parses them
// dynamically adds/removes/shows blocked words
void handle_controller(int tcp_socket)
{
    
    char inMessage[80];
    char outMessage[200];

    printf("Controller connected\n");

    strcat(outMessage, "Hello controller.  Enter:\n"
                        "-'show' to show current blocked words\n"
                        "-'block word#' to add word to the block list\n"
                        "-'unblock word#' to remove word from the block list\n");
    send(tcp_socket, &outMessage, sizeof(outMessage), 0);
    memset(outMessage, 0, sizeof(outMessage));
    size_t recv_size;
    while (1)
    {
        memset(inMessage, 0, sizeof(inMessage));
        memset(outMessage, 0, sizeof(outMessage));

        // recieve the controller message
        recv_size = recv(tcp_socket, &inMessage, sizeof(inMessage), 0);
        if (recv_size == 0){
            printf("controller disconected");
            break;
        }
       
        // showing illegal words
        if(strncmp(inMessage, "show", 4) == 0){
            memset(outMessage, 0, sizeof(outMessage));
            strcat(outMessage, "current blocked words are:\n");
            send(tcp_socket, &outMessage, sizeof(outMessage), 0);

            // iterate through blockList, sending the words to the controller
            for (int i = 0; i < blockList.size(); i++){
                memset(outMessage, 0, sizeof(outMessage));
                strcat(outMessage, blockList[i].c_str());
                send(tcp_socket, &outMessage, sizeof(outMessage), 0);

                memset(outMessage, 0, sizeof(outMessage));
                strcat(outMessage, "\n");
                send(tcp_socket, &outMessage, sizeof(outMessage), 0);                          

            }
        }

        // blocking a word
        if(strncmp(inMessage, "block", 5) == 0){
            // parse the message for the word to block
            string toblock = parseBlock(inMessage);
            memset(outMessage, 0, sizeof(outMessage));
            strcat(outMessage, "word added to block list\n");
            send(tcp_socket, &outMessage, sizeof(outMessage), 0);
            // add the word to blockList
            blockList.push_back(toblock);
        }

        // unblocking a word
        if(strncmp(inMessage, "unblock", 7) == 0){
            printf("unblocking word %s\n", inMessage);
            // parse the message for the word to unblock
            string toUnblock = parseBlock(inMessage);
            // iterate through blocklist, checking each item for a match
            for (int i = 0; i < blockList.size(); i++){
                // if a match is found, erase it
                if(toUnblock == blockList[i]) {
                    blockList.erase(blockList.begin() + (i));
                    memset(outMessage, 0, sizeof(outMessage));
                    strcat(outMessage, "word removed from block list\n");
                    send(tcp_socket, &outMessage, sizeof(outMessage), 0);
                    break;
                }
            }
        }
    }
    close(tcp_socket);
}


int main(){
    // set the message to send to the web server if blocked word is detected
    char blockMessage[2048] = "GET http://pages.cpsc.ucalgary.ca/~carey/CPSC441/ass1/error.html "
                                "HTTP/1.1\r\nHost: pages.cpsc.ucalgary.ca\r\n\r\n";
    
    // create socket variables
	int serverSocket,sControlSocket, clientSocket, cControlSocket;
	// create Address variables
    struct sockaddr_in serverAddress;
	int addressLen = sizeof(serverAddress);
    struct sockaddr_in serverControlAddress;
	int controlAddressLen = sizeof(serverControlAddress);
	// create server and controller sockets
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1){
		printf("Server : failed to create server socket\n");
		return 1;
	}
	printf("Server : server socket created\n");
    sControlSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (sControlSocket == -1){
		printf("Server : failed to create server control socket\n");
		return 1;
	}
	printf("Server : server control socket created\n");
    // Set the server socket to port 8000, the controller socket to port 8001
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(8000);
	printf("Server : server address variables created\n");
    serverControlAddress.sin_family = AF_INET;
	serverControlAddress.sin_addr.s_addr = INADDR_ANY;
	serverControlAddress.sin_port = htons(8001);
	printf("Server : server control address variables created\n");
    // bind the server and controller sockets
	int bindStatus = bind(serverSocket, (struct sockaddr * )&serverAddress, sizeof(serverAddress));
	if(bindStatus == -1){
		printf("Server : unable to bind to server socket\n");
		close(serverSocket);
		return 1;
	}
	printf("Server : bound to socket successfully\n");
    int bindStatus2 = bind(sControlSocket, (struct sockaddr * )&serverControlAddress, sizeof(serverControlAddress));
	if(bindStatus2 == -1){
		printf("Server : unable to bind to server control socket\n");
		close(sControlSocket);
		return 1;
	}
	printf("Server : bound to control socket successfully\n");
    // listen for connections on server and controller sockets
	listen(serverSocket, 5);
	printf("Server : listening for incoming connections\n");
    listen(sControlSocket, 5);
	printf("Server : listening for incoming control connections\n");

    // accept connection on the controller socket
    // note that the controller must connect first
    cControlSocket = accept(sControlSocket, (struct sockaddr *)& serverControlAddress,(socklen_t*)&controlAddressLen);
    // thread the controller socket
    std::thread thread_object(handle_controller, cControlSocket);

	while(1){
		// accept the client connecting to the server socket
        clientSocket = accept(serverSocket, (struct sockaddr *)&serverAddress,(socklen_t*)&addressLen);
        printf("\n\n\n\nServer : connection accepted\n");
        // create the client and webserver message variables
        char clientMessage[2048];
        char webMessage[1000000];
        size_t recv_size;
        while (2){
            // clear web server and client messages
            memset(webMessage, 0, sizeof(webMessage));
            memset(clientMessage, 0, sizeof(clientMessage));
            // recieve the client message
            recv_size = recv(clientSocket, &clientMessage, sizeof(clientMessage), 0);
            if (recv_size == 0){
                printf("Client disconected\n");
                break;
            }
            printf("Client message is\n%s\n", clientMessage);
            // parse the client message for path and host names
            string path = parsePath(clientMessage);
		    string host = parseHost(clientMessage);
            // connect to webserver through new outgoing socket
            int outSocket;
            outSocket = socket(AF_INET, SOCK_STREAM, 0);
		    if (outSocket == -1){
			    printf( "Client : error creating socket\n");
		    }
            // create webAddress variable
            struct sockaddr_in webAddress;
            struct hostent *webpage;
            struct hostent *blockpage;
		    const char * HOST = host.c_str();
            // get IP of hostnames
            webpage = gethostbyname(HOST);
            // check if and illegal messages are in the path
            for (int i = 0; i < blockList.size(); i++){
                // if finds an illegal word
                if (path.find(blockList[i]) != std::string::npos) {
                    printf("illegal word found\n");
                    // redirect the message to get blocked page instead
                    webpage = gethostbyname("pages.cpsc.ucalgary.ca");
                    memset(clientMessage, 0, sizeof(clientMessage));
                    strcpy(clientMessage, blockMessage);
                    break;                
                }               
            }
            // set the webAddress variables
    	    webAddress.sin_family = AF_INET;
		    webAddress.sin_port = htons(80);
		    bcopy(webpage->h_addr, &webAddress.sin_addr, webpage->h_length);
            // Open the outbound socket
		    if(connect(outSocket, (struct sockaddr *)&webAddress, sizeof(webAddress)) == -1){
			    printf( "Client : error connecting to web socket\n");
			    break;	
		    }
            printf("Client : connection to web socket established\n");
            // send the clients message to the web server
            if (send(outSocket, clientMessage, strlen(clientMessage), 0) < 0  ){
   			    printf("Client : message sending failed\n");
			    break;
		    }
            printf("client message forwarded to web server\n");
            // Recieve the response of the web server
            while(recv(outSocket, webMessage, sizeof(webMessage), 0) > 0){
                // forward the response to the web client
                send(clientSocket, webMessage, strlen(webMessage), 0);
                memset(webMessage, 0, sizeof(webMessage));
            }
            printf("web message forwarded to client\n");
            // close the outgoing socket
            close(outSocket);
        }
        // close the client socket
        close(clientSocket);
    }
}