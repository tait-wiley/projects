#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> //strcpy, strlen
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <vector>

using namespace std;

// translate runs on port 5050
#define PORT     5050
#define BUFFSIZE 1024

// translation library
std::vector<std::string> canTranslate {"Hello\r", "Goodbye\r", "One\r", "Two\r", "Three\r"};
std::vector<std::string> translateTo {"Bonjour\r", "Au revoir\r", "Une\r", "Deux\r", "Trois\r"};

// translation
string translate(const char* english_word){
    
    // convert english_word to a string
    std::string word = english_word;
    std::string front = "Translation is: ";

    for (int i = 0; i < canTranslate.size(); i++){
        // if a match is found in library, return translation
        if(word == canTranslate[i]) {
            front += translateTo[i];
            return front;

        }
    }

    // otherwise return that the word is untranslateble
    return "This word cant be translated!\n";

}

// main function
int main() {

    // create socket and buffer variables
    int sockfd;
    char buffer[BUFFSIZE];
    char translated[20];
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

        // recieve word to translate from IS
        n = recvfrom(sockfd, buffer, BUFFSIZE, MSG_WAITALL, ( struct sockaddr *) &client_address, (socklen_t*)&client_address_length);

        printf("%s\n", buffer);

        // get the translated word
        strcpy(translated, translate(buffer).c_str());

        // send translated message to indirectionServer
        sendto(sockfd, translated, strlen(translated), MSG_CONFIRM, (const struct sockaddr *) &client_address, client_address_length);
    }
    
    return 0;
}
