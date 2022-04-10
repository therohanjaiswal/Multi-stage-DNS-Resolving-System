#include <bits/stdc++.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

static const int BUFFER_SIZE = 1024;

/* Function returns the domain name from request buffer */
string getDomainName(char buffer[]) {
	string domain_name;
	if (buffer[2] == 'w' and buffer[3] == 'w' and buffer[4] == 'w' and buffer[5] == '.')
		domain_name = buffer + 6;
	else
		domain_name = buffer + 2;
	domain_name.erase(remove(domain_name.begin(), domain_name.end(), '\n'), domain_name.end());
	return domain_name;
}

/* Function search DNS database to find entry of proxy request */
string searchDatabase(string str, char requestType) {
	ifstream file_pointer("DNS.txt");
	string ip, domain, value;
	bool cache_hit = false;

	// read from DNS.txt
	while (file_pointer >> ip) {
		file_pointer >> domain;
		// if request is Type 1 request
		if(requestType == '1') {
			if(str == domain) {
				value = ip;
				cache_hit = true;
				break;
			}
		}
		// if request is type 2 request
		if(requestType == '2') {
			if (str == ip) {
				value = domain;
				cache_hit = true;
				break;
			}
		} 

	}
	file_pointer.close();

	return cache_hit == true ? value : "";
}

void errorPrompt(const char *err) {
    perror(err);
    exit(1);
}

int main(int argc, char *argv[]) {

	// if the user provide incorrect arguments
	// argument format - filename <Port Number>
	if (argc < 2) 
		errorPrompt("Error, incorrect arguments format given.\n");

	int socket_fd = 0, new_socket_fd = 0;			// File descriptor of socket and new socket
	int address_len = 0, opt = 0;
	int port_no = 0;								// port no
	struct sockaddr_in server_addr, client_addr; 	//	server address and proxy server (client) address
	char read_buffer[BUFFER_SIZE], write_buffer[BUFFER_SIZE];		// buffer for the requested and response message

	// socket creation
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);	
	if(socket_fd < 0) 
		errorPrompt("Socket creation failed.\n");
	else
		fprintf(stderr, "Socket creation successful.\n");

	address_len = sizeof(server_addr);	 			// address length
	port_no = stoi(argv[1]);						// port number 

	// SockOpt, to reuse the address and port and to avoid the error like, "address already in use"
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
		errorPrompt("setsockopt\n");

	// Server Information
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_no);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Binding the socket with the address and port no
	if (bind(socket_fd, (struct sockaddr *)&server_addr, address_len) < 0)
		errorPrompt("Binding failed.\n");

	// Moving to Listen state to listen the request of client
    // At most 5 clients possible
	if (listen(socket_fd, 5) < 0)
		errorPrompt("Listen failed.\n");

multipleClients:
	// accepting and establishing connection from the client
	new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&address_len);
	if(new_socket_fd < 0) 
		errorPrompt("Error, connection not established with the client.\n");
	else 
		fprintf(stderr, "Connection established with the client.\n");

	// new thread creation to handle multiple clients
	int pid = fork();	
	// pid == 0 means child created successfully
	if (pid == 0) {
		// clearing buffer and initializing it with zeros
		bzero(write_buffer, sizeof(write_buffer));
		bzero(read_buffer, sizeof(read_buffer));

		// Reading from client
		int readValue = read(new_socket_fd, read_buffer, sizeof(read_buffer));
		if (readValue < 0)
			errorPrompt("Error reading from socket\n");
		else 
			fprintf(stderr, "Proxy server request: %s\n", read_buffer);
		
		string response;
		// Type 1 request from client i.e., domain -> IP
		if (read_buffer[0] == '1') {
			string domain_name = getDomainName(read_buffer);
			string mapped_ip = searchDatabase(domain_name, read_buffer[0]);

			// cache miss
			if (mapped_ip == "") 
				response = "4 Entry not found in DNS Database.\n";
			// cache hit
			else 
				response = "3 " + mapped_ip;
		}
		// Type 2 request from client i.e., IP -> domain
		else if (read_buffer[0] == '2')	{
			string IP = read_buffer + 2;
			IP.erase(remove(IP.begin(), IP.end(), '\n'), IP.end());
			string mapped_domain = searchDatabase(IP, read_buffer[0]);

			// cache miss
			if(mapped_domain == "") 
				response = "4 Entry not found in DNS Database.\n";
			// cache hit
			else 
				response = "3 www." + mapped_domain + "\n";
		}

		strcpy(write_buffer, response.c_str());
		int writeValue = write(new_socket_fd, write_buffer, strlen(write_buffer));
		if (writeValue < 0)
			errorPrompt("Error writing to the socket.\n");
	} else {	// for more than one client
		goto multipleClients;
	}

	return 0;
}
