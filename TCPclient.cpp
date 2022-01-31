#include <stdio.h> 
#include <stdlib.h> // for standard utilities
#include <sys/socket.h> //for socket function
#include <arpa/inet.h> // for htons and hton1
#include <string.h> // for use of memset
#include <string> // for use of strings
#include <unistd.h> // for close
#include <netdb.h> // for get host by name
#include <cctype> // for isdigit

using namespace std;

// set variables
char sendBuffer[1024]; // buffer for sending
 char replyBuffer[1024]; // buffer for responses from indirection server
char* replyBufferIndex; 
ssize_t amountRecieved; // tracking amount recieved from recv call

// checks if the given cmdline arg is an int between 0-99999
bool is_port(char port[]){

    for (int i = 0; port[i] != 0; i++){
        if (!isdigit(port[i]))
            return false;
        if(i > 4){
            return false;
        }
    }
    return true;
}

// recieves TCP messages from a given socket, stores in replyBuffer
int recieve_message(int socket){
	
	memset(replyBuffer, 0, sizeof(replyBuffer));
	replyBufferIndex = replyBuffer; // resetting reply buffer index
	
	// recieving ALL of server message into replyBuffer, up until null terminator, one byte at a time
    while (1){

		// revieve one amount from the socket into replyBuffer
		amountRecieved = recv(socket, replyBufferIndex, 1, 0);

		// check if server has disconnected
		if (amountRecieved == 0){
			printf("Server disconnected\n");
			close(socket);
			return 0;
		} 

		// check for recieving error
		if (amountRecieved == -1){
			printf("error recieving server message\n");
			close(socket);
			return -1;
		}
    
		// check for the terminating character
		if (strchr(replyBuffer, '\r') != NULL){
        	return 1;
    	}

		// if no terminating character
    	else{
        	//fill the next chunk of the buffer
        	replyBufferIndex += amountRecieved;
    	}	
	}// end while
}// end function

// gets client input, sends to the server
void send_input(int socket){
		
	memset(sendBuffer, 0, sizeof(sendBuffer));

	// prompt for input
	printf("need input: ");
	scanf("%s", sendBuffer);

	// terminate the input
	sendBuffer[strlen(sendBuffer)] = '\r';

	// send the input to the server
	send(socket, sendBuffer, strlen(sendBuffer), 0);
	printf("Input sent to indirection server\n");
}

// encryptes a given ID with a given Key
int encrypt(int ID, int Key){
	return ID * Key;
}

// run via the command line by ./filename PORT IP
// where PORT IP are those of the indirection server
int main(int argc, char **argv){
	// initialize encryption variables
	int key;
	int vote;
	int encrypted_vote;
	char* pEnd;

    // make sure there are 2 command line arguments
    if(argc != 3){
        printf("Please give the Port and IP address as cmd line arguments\n");
        return 0;
    }

    // check if valid port number
    if(!is_port(argv[1]) ){
        printf("Please give a valid port number\n");
        return 0;
    }

	// creat a socket variable
	int outSocket;
	    
    // new sockaddr_in object for the server
	struct sockaddr_in servAddress;
	memset(&servAddress, 0, sizeof(servAddress));

	// set servAddress variables
	servAddress.sin_family = AF_INET;
	servAddress.sin_port = htons(atoi(argv[1]));
	servAddress.sin_addr.s_addr = inet_addr(argv[2]);

    // create outgoing TCP socket
	outSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (outSocket == -1){
		printf( "Client : error creating socket\n");
	}

	printf( "Client : Socket created\n");

	// Sending connect request to the indirection server (at address given)
	if(connect(outSocket, (struct sockaddr *)&servAddress, sizeof(servAddress)) == -1){
		printf( "Client : error connecting to server socket\n");
		return 0;	
	}
		
	printf("Client : connection to indirection server socket established\n");


	// server interaction loop, recieve a message, then scan for markers to see what to do next
	for(;;){

		// revieve a TCP message from the socket
		int a = recieve_message(outSocket);
		if (a == 0){
        	return 0;
    	}
    	if (a == -1){
        	return 0;
    	}
        
		// display message to user
    	printf("%s\n", replyBuffer);

		// if message is a encryption key
		if(strstr(replyBuffer, "\t\t\t\r") != NULL){
			
			// get the key
			key = strtol(replyBuffer, &pEnd, 10);

			printf("Key is above\n");

			printf("what is the ID of the candidate you wish to vote for?\n");

			// get input integer of who they want to vote for
			scanf("%d", &vote);

			// perform encryption function 
			encrypted_vote = encrypt(vote, key);

			printf("encrypted vote is %d\n", encrypted_vote);
			
			// put encrypted vote in sendBuffer, terminate with \r, then send to server
			memset(sendBuffer, 0, sizeof(sendBuffer));
			strcat(sendBuffer, to_string(encrypted_vote).c_str());
			sendBuffer[strlen(sendBuffer)] = '\r';
			send(outSocket, sendBuffer, strlen(sendBuffer), 0);

			printf("sent encrypted vote to indirection server\n");
		}

		// if the signal to end the session is seen, end the session
		else if(strstr(replyBuffer, "\t\t\r") != NULL){
			printf("ending session\n");
			close(outSocket);
			return 0;
		}

		// if this messsage also requests input, get the input then send to server
		else if(strstr(replyBuffer, "\t\r") != NULL){
			send_input(outSocket);
		}

	} // end of for loop, client/server loop

	close(outSocket);
	return 0;
}

