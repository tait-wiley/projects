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
#include <array>
#include <math.h> // for roundf

using namespace std;

// currency translations
//converting the Canadian Dollar to US Dollar, Euro, British Pound, and Bitcoin currencies
// 1 CAN = .81 US
// 1 CAN = .70 EURO
// 1 CAN = .59 POUND
// 1 CAN = 0.000016 BITCOIN

// translate runs on port 5051
#define PORT     5051
#define BUFFSIZE 1024

// translation library
std::vector<std::string> canConvert {"US", "EURO", "POUND", "BITCOIN"};
std::array<float, 4> convertNum {.81, .70, .59, 0.000016};

// function to convert the currency
// converts from CAD amount, to destination_country amount
string convert(const char* destination_country, float amount){

    // create floats for amounts
    float Total;
    float rounded;

    string toReturn = "Conversion is: ";
    
    // convert english_word to a string
    std::string destination = destination_country;

    for (int i = 0; i < canConvert.size(); i++){
        // if a match is found in library, return translation
        if(destination == canConvert[i]) {
            
            Total = (amount * convertNum[i]);
            // round to 2 decimal places
            rounded = roundf(Total * 100) / 100;             
            toReturn += to_string(rounded);
            toReturn += " ";
            toReturn += canConvert[i];
            toReturn += '\r';
            return toReturn;

        }
    }
    // otherwise return that it's untranslateble (couldnt match destination country)
    return "This cant be converted!\r";

}

// main function
int main() {
    int sockfd;
    char buffer[BUFFSIZE];
    char converted[20];
    char country[20];
    float amount;
    char* pEnd;
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

        // get the UDP message from the IS
        n = recvfrom(sockfd, buffer, BUFFSIZE, MSG_WAITALL, ( struct sockaddr *) &client_address, (socklen_t*)&client_address_length);

        // parse the amount the client wishes to convert
        amount = strtof(buffer, &pEnd);

        // if client wants US
        if (strstr(buffer, "US") != NULL){            
            strcpy(converted, convert("US", amount).c_str());
            
        }

        // if client wants EURO
        else if (strstr(buffer, "EURO") != NULL){
            strcpy(converted, convert("EURO", amount).c_str());
        }

        // if client wants POUND
        else if (strstr(buffer, "POUND") != NULL){
            strcpy(converted, convert("POUND", amount).c_str());
        }

        // if client wants BITCOIN
        else if (strstr(buffer, "BITCOIN") != NULL){
            strcpy(converted, convert("BITCOIN", amount).c_str());
        }

        // if appropriate destination not matched
        else{
            strcpy(converted, convert("NULL", amount).c_str());
        }

        // send converted amount to IS
        sendto(sockfd, converted, strlen(converted), MSG_CONFIRM, (const struct sockaddr *) &client_address, client_address_length);

    }   

    return 0;
}
