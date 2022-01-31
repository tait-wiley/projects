#include <stdio.h>
#include <stdlib.h> // standard utilities
#include <unistd.h> // close
#include <string.h> //strcpy, strlen
#include <string> // string functions
#include <sys/types.h> // types
#include <sys/socket.h> // for suckets
#include <arpa/inet.h> // for htons
#include <netinet/in.h> // for UDP 
#include <sys/time.h> // for UDP timeouts

using namespace std;

// Buffer variables
char sendBuffer[1024];
char responseBuffer[1024];
char* responseBufferIndex;
ssize_t amountRecieved;

struct timeval timeout={2,0}; //set timeout for 2 seconds

// keeping track if client has voted
bool hasVoted = false;

// inbound TCP server port running on port 8000
#define TCPPORT 8000
// UDP ports
#define UDPtranslate 5050
#define UDPconvert 5051
#define UDPvote 5052
// size of buffer
#define BUFFSIZE 1024

// revieving a TCP message from a socket
int recieve_message(int socket){

	memset(responseBuffer, 0, sizeof(responseBuffer));
    // resetting reply buffer index
	responseBufferIndex = responseBuffer; 
	
	// recieving ALL of server message into responseBuffer, up until null terminator
    while (1){

        // reviece one byte from the socket into responseBuffer
		amountRecieved = recv(socket, responseBufferIndex, 1, 0);

        // check if the client disconnected
		if (amountRecieved == 0){
			printf("Client disconnected\n");
			close(socket);
			return 0;
		} 

        // Check if there was an error recieving the client's message
		if (amountRecieved == -1){
			printf("error recieving client message\n");
			close(socket);
			return -1;
		}    	

        // Check if the string terminator has been reached
		if (strchr(responseBuffer, '\r') != NULL){
        	return 1;
    	}

        // If the terminator has not been reached
    	else{
        	//fill the next chunk of the buffer
        	responseBufferIndex += amountRecieved;
    	}	
	}
}

// sends a string to a UDP server at a port, returns the string response
string UDPsend_recieve(string word, int port){

    // create socket and buffer variables
    int sockfd;
    char buffer[BUFFSIZE];


    memset(buffer, 0, sizeof(buffer));

    // copy the input string into a newly created message variable
    char message[word.size() + 1];
    strcpy(message, word.c_str()); 

    // create address structure
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
  
    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
  
    // Filling server information
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    // UDP servers run from localhost
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
      
    // n = total bytes recieved  
    int n, server_address_length; 
      
    // send word to translator server  
    sendto(sockfd, message, strlen(message),
        MSG_CONFIRM , (const struct sockaddr *) &server_address, 
            sizeof(server_address));

    printf("message sent to UDP microserver\n");


    // implement setsockopt() for a timeout


    setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));
    // recieve response from translator server     
    n = recvfrom(sockfd, (char *)buffer, BUFFSIZE, 
                MSG_WAITALL, (struct sockaddr *) &server_address,
                (socklen_t *)&server_address_length);
    if(n >= 0){
        printf("response from microserver is: %s\n", buffer);
  
        // set translated string to word in buffer
        string response = string(buffer);

        // close the socket
        close(sockfd);

        // return the response
        return response;

    }

    else{
        return "UDP server did not answer in time\r";
    }
}

// send and recieve a UDP response to the UDPtranslate server
string translator(string word){
    return UDPsend_recieve(word, UDPtranslate);
   
}

// send and recieve a UDP response to the UDPconvert server
string converter(string word){
    return UDPsend_recieve(word, UDPconvert);
}

// send and recieve a UDP response to the UDPvote server
string voter(string word){
    return UDPsend_recieve(word, UDPvote);
}

// start of main
int main() {

    // TCP connection
    // initializing the 2 address variables
	struct sockaddr_in server;
	struct sockaddr_in client;

	// initializing the 2 socket variables
	int serverSocket;
	int clientSocket;

    // pre defined messages, \r included to prompt for user input
    char introMessage[1024] = "\nWelcome to the indirection server:\n"
                            "- For the translator service, enter '1':\n"
                            "- For the currency service, enter '2':\n"
                            "- For the voting service, enter '3':\n"
                            "- For ending the session, enter '4':\t\r";

    char translateMessage[1024] = "\nWhat word would you like translated?:\t\r";
    char convertMessage1[1024] = "\nHow many canadian dollars whould you like converted?:\t\r";
    char convertMessage2[1024] = "\nWhat currency would you like it converted to? (US, EURO, POUND, BITCOIN):\t\r";

    char voteOptions[1024] = "\nWelcome to the voting service:\n"
                            "- To see the list of available candidates, enter '1':\n"
                            "- To register a vote, enter enter '2':\n" 
                            "- To see the current vote tally, enter '3':\n"
                            "- To return to the previous menu, enter '4':\t\r";

    char vote[1024] = "Enter the ID of the candidate you wish to vote for:\t\r";
    char earlyResults[1024] = "You must vote before seeing the results!\r";
    char endMessage[1024] = "\t\t\r";

	// create the server socket
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1){
		printf("Server : failed to create server socket\n");
		return 1;
	}

	printf("Server : server socket created\n");

	// setting the server sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(TCPPORT);  

	// binding to the serversocket with the information provided byt the server sockaddr_in structure
	int bindStatus = bind(serverSocket, (struct sockaddr * )&server, sizeof(server));
	if(bindStatus == -1){
		printf("Server : unable to bind to server socket\n");
		close(serverSocket);
		return 1;
	}

	printf("Server : bound to socket successfully\n" );
	
	// listen for connections on the socket, with 5 allowable client connections in backlog
	listen(serverSocket, 5);

	printf("Server : waiting for clients to connect to the socket\n");

	// accept a connection from a client.
	clientSocket = accept(serverSocket, NULL, NULL);
	if(clientSocket == -1){
		printf("Server : client connection failed\n");
		return 1;
	
	}

	printf("Server : client connection accepted\n");

	//setting up the message handling loop
	for(;;){

        // send the intro menu to the client
        send(clientSocket, introMessage, strlen(introMessage), 0);

        // client response stores in response buffer
        int a = recieve_message(clientSocket);
        if (a == 0){
            return 0;
        }
        if (a == -1){
            return 0;
        }

        printf("client response is %s\n", responseBuffer);

        // if the client wants to translate a word
        if(responseBuffer[0] == '1'){
            printf("client selected 1!\n");

            // prompt for translation input
            send(clientSocket, translateMessage, strlen(translateMessage), 0);
            // recieve the input
            int b = recieve_message(clientSocket);
            if (b == 0){
                return 0;
            }
            if (b == -1){
                return 0;
            }
            
            // put the client input in sendBuffer
            memset(sendBuffer, 0, sizeof(sendBuffer));
            strcpy(sendBuffer, responseBuffer);

            // put the translation from the UDP server in responseBuffer
            memset(responseBuffer, 0, sizeof(responseBuffer));
            strcpy(responseBuffer, (translator(sendBuffer)).c_str());

            // send the translated message to the client
            send(clientSocket, responseBuffer, strlen(responseBuffer), 0);

        }

        // if the client wants to convert canadian dollars
        else if(responseBuffer[0] == '2'){
            printf("client selected 2!\n");

            // prompt the client for conversion ammount
            send(clientSocket, convertMessage1, strlen(convertMessage1), 0);

            // recieve the amount
            int c = recieve_message(clientSocket);
            if (c == 0){
                return 0;
            }
            if (c == -1){
                return 0;
            }
            
            // prompt the client for what country currency they want the amount converted to
            memset(sendBuffer, 0, sizeof(sendBuffer));
            strcpy(sendBuffer, responseBuffer);
            strcat(sendBuffer, " ");
            send(clientSocket, convertMessage2, strlen(convertMessage2), 0);

            // recieve the country
            int d = recieve_message(clientSocket);
            if (d == 0){
                return 0;
            }
            if (d == -1){
                return 0;
            }

            // get the conversion from the UDP server
            strcat(sendBuffer, responseBuffer);
            memset(responseBuffer, 0, sizeof(responseBuffer));
            strcpy(responseBuffer, converter(sendBuffer).c_str());

            // send the conversion to the client
            send(clientSocket, responseBuffer, strlen(responseBuffer), 0);
        } 

        // if the client wants access to the voting service
        else if(responseBuffer[0] == '3'){
            printf("client selected 3!\n");

            // start the voting loop
            while(1){
                
                // send the client their various voting options
                send(clientSocket, voteOptions, strlen(voteOptions), 0);

                // get the clients option
                int e = recieve_message(clientSocket);
                if (e == 0){
                    return 0;
                }
                if (e == -1){
                    return 0;
                }

                // client wants to go back to the previous menu
                if(responseBuffer[0] == '4'){
                    memset(responseBuffer, 0, sizeof(responseBuffer));
                    break;
                }

                // client wants to see the list of candidates
                else if(responseBuffer[0] == '1'){
                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    strcpy(sendBuffer, responseBuffer);
                    memset(responseBuffer, 0, sizeof(responseBuffer));

                    // send request for candidates to the UDP server, get response
                    strcpy(responseBuffer, voter(sendBuffer).c_str());

                    // send the candidates to the client
                    send(clientSocket, responseBuffer, strlen(responseBuffer), 0);
                }

                // client wants to see the voting results
                else if(responseBuffer[0] == '3'){

                    // check if the client has voted
                    // if they havnt, they cant see the results
                    if(hasVoted == false){
                        send(clientSocket, earlyResults, strlen(earlyResults), 0);
                    }

                    // if they have voted
                    else{
                        memset(sendBuffer, 0, sizeof(sendBuffer));
                        strcpy(sendBuffer, responseBuffer);
                        memset(responseBuffer, 0, sizeof(responseBuffer));

                        // send request for current results to UDP server, get response
                        strcpy(responseBuffer, voter(sendBuffer).c_str());

                        // send the results to the client
                        send(clientSocket, responseBuffer, strlen(responseBuffer), 0);
                    }
                }

                // client wishes to enter a vote
                else if(responseBuffer[0] == '2'){

                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    strcpy(sendBuffer, responseBuffer);
                    memset(responseBuffer, 0, sizeof(responseBuffer));

                    // send key request to microserver, get key response
                    strcpy(responseBuffer, voter(sendBuffer).c_str());

                    // foward the key to client
                    send(clientSocket, responseBuffer, strlen(responseBuffer), 0);

                    // recieve the encrypted vote from the client
                    int f = recieve_message(clientSocket);
                    if (f == 0){
                        return 0;
                    }
                    if (f == -1){
                        return 0;
                    }

                    memset(sendBuffer, 0, sizeof(sendBuffer));
                    
                    // send encrypted vote to microserver, get the confirmation message
                    strcpy(sendBuffer, voter(responseBuffer).c_str());

                    // client has now voted, update hasVoted and allow client to see voting results
                    if(sendBuffer[0] != 'P'){
                        hasVoted = true;
                    }

                    // send the confirmation message to the client
                    send(clientSocket, sendBuffer, strlen(sendBuffer), 0);               
                }
             
            } // end voting loop

        }// end of voting service 

        // client wants to close session
        else if(responseBuffer[0] == '4'){
            printf("client selected 4!\n");
            printf("ending session\n");

            // send the end session string to the client
            send(clientSocket, endMessage, strlen(endMessage), 0);
            // close both sockets
            close(clientSocket);
	        close(serverSocket);
            // shutdown the server
            return 0;

        }            

	} //end client server loop
	
    // closing the sockets
	close(clientSocket);
	close(serverSocket);
}
