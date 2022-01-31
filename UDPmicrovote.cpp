#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <vector>
#include <ctime>


using namespace std;

// vote runs on port 5052
#define PORT     5052
#define BUFFSIZE 1024

// translation librarys
std::vector<std::string> candidates {"Jeff", "Doug", "Carly", "Francine", "Bob"};
std::vector<int> IDs {1, 2, 3, 4, 5};
std::vector<int> votes {11, 22, 58992, 44, 55};

// encryption variables
int key;
int encrypted;
int decrypted;
char* pEnd;

// checking if client voted for a valid ID
bool validID;

// returning the list of candidates
string return_candidates(){
    
    string candidates_ids = "The candidates are:\n";

    for (int i = 0; i < candidates.size(); i++){
        candidates_ids += candidates[i];
        candidates_ids += ": ID = ";
        candidates_ids += to_string(IDs[i]);
        candidates_ids += ". ";
    }
    
    // terminate the list with \r
    candidates_ids += '\r';

    return candidates_ids;
}

// returning the voting results
string return_results(){
    
    string results = "The current results are:\n";

    for (int i = 0; i < candidates.size(); i++){
        results += candidates[i];
        results += ": ";
        results += to_string(votes[i]);
        results += " votes. ";
    }
    // terminate the results with \r
    results += '\r';

    return results;
}

// generate a random key between 1 and 10
int randomNum(){
    srand(time(NULL));
    return (rand() % 10) + 1;
}

// start of main
int main() {

    // create socket and buffer variables
    int sockfd;
    char buffer[BUFFSIZE];
    char sendBuffer[1024];
    struct sockaddr_in server_address, client_address;
    
    // n is total bytes recieved
    int client_address_length, n; 
    client_address_length = sizeof(client_address); 

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_address, 0, sizeof(server_address));
    memset(&client_address, 0, sizeof(client_address));

    // Filling server information
    server_address.sin_family = AF_INET; // IPv4
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&server_address, sizeof(server_address)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    printf("bound to socket\n"); 

    while (true){

        memset(buffer, 0, sizeof(buffer));

        // recieve a UDP message from the indirection server
        n = recvfrom(sockfd, buffer, BUFFSIZE, MSG_WAITALL, ( struct sockaddr *) &client_address, (socklen_t*)&client_address_length);
      
        // client wants list of candidates
        if (buffer[0] == '1'){
            
            memset(sendBuffer, 0, sizeof(sendBuffer));
            
            // get the list of candidates
            strcpy(sendBuffer, return_candidates().c_str());            

            // send list to indirection server
            sendto(sockfd, sendBuffer, strlen(sendBuffer), MSG_CONFIRM, (const struct sockaddr *) &client_address, client_address_length);
        }

        // client wants to vote
        else if (buffer[0] == '2'){
            
            // generate an encryption key between 1 and 10
            key = randomNum();
            memset(sendBuffer, 0, sizeof(sendBuffer));
            strcpy(sendBuffer, (to_string(key).c_str()));

            // signify this message is a key
            strcat(sendBuffer, "\t\t\t\r");

            // send key to IS
            sendto(sockfd, sendBuffer, strlen(sendBuffer), MSG_CONFIRM, (const struct sockaddr *) &client_address, client_address_length);
            
            // recieve encrypted message from IS
            memset(buffer, 0, sizeof(buffer));
            n = recvfrom(sockfd, buffer, BUFFSIZE, MSG_WAITALL, ( struct sockaddr *) &client_address, (socklen_t*)&client_address_length);

            // decrypt ID to add one more vote for the candidate 
            encrypted = strtol(buffer, &pEnd, 10);
            decrypted = static_cast<int>(encrypted / key);
        
            // add one vote to the ID
            for (int i = 0; i < IDs.size(); i++){
                if (IDs[i] == decrypted){
                    votes[i] += 1;
                    // mark the ID as valid
                    validID = true;
                }
            }

            // send confirmation message to IS
            memset(sendBuffer, 0, sizeof(sendBuffer));

            // if ID was valid
            if(validID == true){
                strcat(sendBuffer, "Vote recorded!\r");
            }

            // if ID was not valid
            else{
                strcat(sendBuffer, "Please enter a valid ID!\r");
            }
            
            sendto(sockfd, sendBuffer, strlen(sendBuffer), MSG_CONFIRM, (const struct sockaddr *) &client_address, client_address_length);

            // reset validID
            validID = false;
        }

        // client wants current vote results
        else if (buffer[0] == '3'){

            memset(sendBuffer, 0, sizeof(sendBuffer));

            // get the results, send them to IS
            strcpy(sendBuffer, return_results().c_str());         
            sendto(sockfd, sendBuffer, strlen(sendBuffer), MSG_CONFIRM, (const struct sockaddr *) &client_address, client_address_length);           
        }       

    } // end of while loop   

    return 0;
}
