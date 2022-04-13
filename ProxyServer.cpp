#include <bits/stdc++.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

static const int CACHE_SIZE = 3;
static const char DNS_IP[100] = "127.0.0.1";
static const int BUFFER_SIZE = 1024;

/* Fucnction which shows error */
void errorPrompt(const char *err)
{
    perror(err);
    exit(1);
}

/* Function returns the domain name from request buffer */
string getDomainName(char buffer[])
{
    string domain_name;
    if (buffer[2] == 'w' and buffer[3] == 'w' and buffer[4] == 'w' and buffer[5] == '.')
        domain_name = buffer + 6;
    else
        domain_name = buffer + 2;
    domain_name.erase(remove(domain_name.begin(), domain_name.end(), '\n'), domain_name.end());
    return domain_name;
}

/* Function searches the client request in Proxy server cache*/
string cacheCheck(string str, char requestType)
{
    ifstream file_pointer("proxyCache.txt", ifstream::in);
    string ip, domain, value;
    bool cache_hit = false;

    // read proxyCache.txt
    while (file_pointer >> ip >> domain)
    {
        //  if request is type 1 request
        if (requestType == '1')
        {
            if (str == domain)
            {
                value = ip;
                cache_hit = true;
                break;
            }
        }
        // if request is type 2 request
        if (requestType == '2')
        {
            str = str.substr(0, str.length() - 1);
            if (str == ip)
            {
                value = domain;
                cache_hit = true;
                break;
            }
        }
    }
    file_pointer.close();
    return cache_hit == true ? value : "";
}

/* Function sends request to DNS server from Proxy server */
string requestToDNS(char buffer[])
{
    int socket_fd = 0, new_socket_fd = 0;
    int DNS_port_no = 0;
    int addr_len = 0;
    struct sockaddr_in dns_serv_addr;
    char dns_request_buffer[BUFFER_SIZE], dns_response_buffer[BUFFER_SIZE], temp_buffer[BUFFER_SIZE];

    // create the socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
        errorPrompt("Socket creation failed.\n");
    else
        fprintf(stderr, "Socket creation successful.\n");

    // Take DNS port as input
    fprintf(stderr, "Enter DNS server port number: ");
    cin >> DNS_port_no;
    addr_len = sizeof(dns_serv_addr);

    // DNS Server Information
    dns_serv_addr.sin_family = AF_INET;
    dns_serv_addr.sin_port = htons(DNS_port_no);
    inet_aton(DNS_IP, (struct in_addr *)&dns_serv_addr.sin_addr.s_addr);

    // connect with the DNS server
    if (connect(socket_fd, (struct sockaddr *)&dns_serv_addr, addr_len) < 0)
        errorPrompt("Error, Connection not established with DNS.\n");
    else
        fprintf(stderr, "Connection established with DNS server.\n");

    // clearing buffer and initializing it with zeros
    bzero(dns_request_buffer, sizeof(dns_request_buffer));
    bzero(dns_response_buffer, sizeof(dns_response_buffer));
    bzero(temp_buffer, sizeof(temp_buffer));

    // Write DNS request
    fprintf(stderr, "Sending DNS request...\n");
    strcpy(dns_request_buffer, buffer);
    int writeValue = write(socket_fd, dns_request_buffer, strlen(dns_request_buffer));
    if (writeValue < 0)
        errorPrompt("Error writing to socket\n");

    // Read DNS response
    int readValue = read(socket_fd, dns_response_buffer, sizeof(dns_response_buffer));
    if (readValue < 0)
        errorPrompt("Error reading from socket.\n");
    else
        fprintf(stderr, "DNS server response: %s\n", dns_response_buffer);

    // Type 3 response from DNS
    if (dns_response_buffer[0] == '3')
    {
        string ip, domain, line;
        vector<string> cacheData;

        strcpy(temp_buffer, dns_response_buffer + 2);

        // Type 1 request: Domain -> IP
        if (buffer[0] == '1')
        {
            domain = getDomainName(buffer);
            ip = temp_buffer;
        }
        // Type 2 request: IP -> Domain
        else if (buffer[0] == '2')
        {
            ip = buffer + 2;
            ip.erase(remove(ip.begin(), ip.end(), '\n'), ip.end());
            domain = temp_buffer;
        }

        // Updating the proxy server cache using FIFO
        ifstream filepointer("proxyCache.txt");
        while (getline(filepointer, line))
            cacheData.push_back(line);

        string new_entry = ip + " " + domain;
        if (cacheData.size() < 3)
        { // cache is not full
            cacheData.push_back(new_entry);
        }
        else
        { // cache is full
            for (int i = 0; i < CACHE_SIZE - 1; ++i)
                cacheData[i] = cacheData[i + 1];
            cacheData[CACHE_SIZE - 1] = new_entry;
        }

        // Update proxyCache.txt
        ofstream out;
        out.open("proxyCache.txt", ofstream::out | ofstream::trunc);
        int i = 0;
        while (i < cacheData.size() - 1)
        {
            out << cacheData[i] << "\n";
            ++i;
        }
        out << cacheData[i];
        out.close();

        if (buffer[0] == '1')
        {
            return "Associated IP address: " + ip + "\n";
        }
        else if (buffer[0] == '2')
        {
            return "Associated domain name: www." + domain + "\n";
        }
    }
    // when even DNS database does not contain the required information
    else if (dns_response_buffer[0] == '4')
    {
        return "Error, Entry not found in DNS Database!\n";
    }
    return "Invalid response from DNS server.\n";
}

int main(int argc, char *argv[])
{

    // if the user provide incorrect arguments
    // argument format - filename <Port Number>
    if (argc < 2)
    {
        errorPrompt("Error, incorrect arguments format given.\n");
    }

    int socket_fd = 0, new_socket_fd = 0;                     // File descriptor for the socket (0 : if created, -1 : if failed)
    int port_no = 0;                                          // Port no
    struct sockaddr_in server_addr, client_addr;              // Server (Proxy server) and client address
    int addr_len = 0, opt = 0;                                // Lengtho or size of the address
    char read_buffer[BUFFER_SIZE], write_buffer[BUFFER_SIZE]; // buffer for the requested and response message

    // socket creation
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        errorPrompt("socket creation failed.\n");
    }
    else
    {
        fprintf(stderr, "Socket creation successful.\n");
    }

    port_no = stoi(argv[1]);
    addr_len = sizeof(server_addr);

    // SockOpt, to reuse the address and port and to avoid the error like, "address already in use"
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        errorPrompt("Setsockopt");

    // Server Information
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_no);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Socket binding
    if (bind(socket_fd, (struct sockaddr *)&server_addr, addr_len) < 0)
        errorPrompt("Error, Binding failed.\n");

    // Moving to Listen state to listen the request of client
    // At most 5 clients possible
    if (listen(socket_fd, 5) < 0)
        errorPrompt("Error, Listen failed.\n");

multipleClients:
    // Accepting client request
    new_socket_fd = accept(socket_fd, (struct sockaddr *)&client_addr, (socklen_t *)&addr_len);
    if (new_socket_fd < 0)
    {
        perror("Error, connection not established.\n");
    }

    // new thread creation to handle multiple clients
    int pid = fork();
    // pid == 0 means child created successfully
    if (pid == 0)
    {
        // clearing buffer and initializing it with zeros
        bzero(read_buffer, sizeof(read_buffer));
        bzero(write_buffer, sizeof(write_buffer));

        // read client request
        int readValue = read(new_socket_fd, read_buffer, sizeof(read_buffer));
        if (readValue < 0)
            errorPrompt("Error reading from socket\n");
        else
            fprintf(stderr, "Request from Client: %s", read_buffer);

        string response;
        // Type 1 request: domain name -> ip address
        if (read_buffer[0] == '1')
        {
            string domain_name = getDomainName(read_buffer);
            string mapped_ip = cacheCheck(domain_name, read_buffer[0]);

            // cache miss
            if (mapped_ip == "")
            {
                fprintf(stderr, "Cache miss. Connecting with DNS Server...\n");
                strcpy(write_buffer, requestToDNS(read_buffer).c_str()); // communicate with DNS Server
            }
            // cache hit
            else
            {
                response = "Cache hit. Proxy server response: " + mapped_ip + "\n";
                strcpy(write_buffer, response.c_str());
            }
        }
        // Type 2 request: ip address -> domain name
        else if (read_buffer[0] == '2')
        {
            string IP = read_buffer + 2;
            string mapped_domain = cacheCheck(IP, read_buffer[0]);
            // cache miss
            if (mapped_domain == "")
            {
                fprintf(stderr, "Cache miss. Connecting with DNS Server...\n");
                strcpy(write_buffer, requestToDNS(read_buffer).c_str()); // communicate with the DNS Server
            }
            // cache hit
            else
            {
                response = "Cache hit. Proxy server response: " + mapped_domain + "\n";
                strcpy(write_buffer, response.c_str());
            }
        }
        else
        { // Invalid client request
            response = "Invalid client request! Try again.\n";
            strcpy(write_buffer, response.c_str());
        }

        // Sending response to client
        int writeValue = write(new_socket_fd, write_buffer, strlen(write_buffer));
        if (writeValue < 0)
        {
            errorPrompt("Error writing to the socket.\n");
        }
    }
    else
    { // for more than one client
        goto multipleClients;
    }
    close(socket_fd);

    return 0;
}
