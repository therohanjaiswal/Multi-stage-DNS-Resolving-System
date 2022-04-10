#include <bits/stdc++.h>
#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

using namespace std;

static const int BUFFER_SIZE = 1024;

void errorPrompt(const char *err) {
    perror(err);
    exit(1);
}

int main(int argc, char *argv[]){

    // if the user provide incorrect arguments
    // argument format - filename <IP adress> <Port Number>
    if(argc < 3) 
        errorPrompt("Error, incorrect arguments format given.\n");

    int socket_fd = 0;                          // File descriptor for the socket (0 : if created, -1 : if failed)
    int addr_len = 0;                           // Address length
    int port_no = 0;                            // Port number
    char *serverIP;                             // server IP
    char write_buffer[BUFFER_SIZE], read_buffer[BUFFER_SIZE];   // buffer for the request and response message
    struct sockaddr_in server_addr;                             // Server's address (Proxy server)

    // Create socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);    
    if (socket_fd < 0) 
        errorPrompt("Socket creation failed.\n"); 
    else  
        fprintf(stderr, "Socket creation successful.\n");
    
    addr_len = sizeof(server_addr);                 // Length or size of the address
    port_no = stoi(argv[2]);                        // Port number from the command line
    serverIP = argv[1];                             // server's ip address from the comand line



    // Server Information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_no);
    inet_aton(serverIP, (struct in_addr *) &server_addr.sin_addr.s_addr);

    // Requesting for connection with the server
    if(connect(socket_fd, (struct sockaddr *) &server_addr, addr_len) < 0)
        errorPrompt("Error, connection not established with the server.\n");
    else
        fprintf(stderr, "Connection established with the server.\n");
        
    // Communication: DNS Resolving System
    // Type 1 request format - 1 www.abcd.com 
    // Type 2 request format - 2 1.1.1.1
    fprintf(stderr, "Type your request in standard format:-\n");
    fprintf(stderr, "Type 1 request format: 1 www.abcd.com\n");
    fprintf(stderr, "Type 2 request format: 2 1.1.1.1\n");
    bzero(write_buffer, sizeof(write_buffer));        // Clearing the buffer and setting it to 0's
    fgets(write_buffer, sizeof(write_buffer), stdin); // read client request from terminal
    int writeValue = write(socket_fd, write_buffer, strlen(write_buffer)); // send that message to the proxy server
    if(writeValue < 0){
        errorPrompt("Error writing to socket.\n");
    }

    // Read server response
    bzero(read_buffer, sizeof(read_buffer));                            // Clearing the buffer and setting it to 0's
    int readValue = read(socket_fd, read_buffer, sizeof(read_buffer));  // Read the information (message) sent by the server
    if (readValue < 0) {
        errorPrompt("Error reading from socket.\n");
    } else {
        fprintf(stderr, "Response from Server :- %s", read_buffer);
    }

    return 0;
}
