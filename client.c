/* 
    This is a simple TCP client written in the C programming language. 
    The program is written and tested on Ubuntu. Furthermore, this 
    code is IP version-agnostic. 
            
        Ubuntu version:         22.04.3 LTS
        lippcap version:        1.10.1
        C/C++ compiler version: GCC 11.4.0
        Linux kernel version:   5.15.90.1-microsoft-standard-WSL2

    Author: Ravindu Karunathilake   
    Date:   2024/03/25
*/
#include <stdlib.h> // exit()
#include <unistd.h> // fork() and close() 
#include <errno.h> 
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <signal.h>

#include <getopt.h>
static struct option long_options[] = {
    {"server_hostname", required_argument, 0, 'i'},
    {"server_port", required_argument, 0, 'p'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};

#include "utility.h" 

#define MAXDATASIZE 100 // Maximum number of bytes to be received 

// Function prototypes 
void print_usage();

int main(int argc, char *argv[]){
    // Check correct number of arguments provided 
    if (argc == 1){
        print_usage();
        exit(EXIT_SUCCESS);
    }
    
    int option;
    while((option = getopt_long(argc, argv, "i:p:h", long_options, NULL)) != -1){
        switch(option){
            case 'i':
                #define HOSTNAME = optarg;
                break; 
            case 'p':
                #define PORT = optarg; 
                break;
            case 'h':
                print_usage();
                exit(EXIT_SUCCESS);
            default:
                printf("Invalid option or missing argument. Use -h or --help for usage.\n");
                exit(EXIT_FAILURE);
        }   
    }

    int sock_fd;
    char buffer[MAXDATASIZE]; 
    struct addrinfo hints; 
    struct addrinfo *clientinfo_res; 

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;

    int status = -1;
    // argv[1] - server host name, argv[2] - server port number
    if((status = getaddrinfo(argv[1], argv[2], &hints, &clientinfo_res)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    
    printf("Client IP addresses are:\n\n");
    // Create the first available socket and connect to remote server 
    struct addrinfo *p;
    for(p = clientinfo_res; p != NULL; p = p->ai_next){
        // Make a socket
        if((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("client: socket creation");
            continue;
        }

        // Print the socket address information to STDOUT
        sockAddrPrint(p);

        // Connect to the remote server 
        if(connect(sock_fd, p->ai_addr, p->ai_addrlen) == -1){
            close(sock_fd);
            perror("client: connect");
            continue;
        }
        break;
    }

    // Free the linked list populated by 'getaddrinfo()' call
    freeaddrinfo(clientinfo_res); 

    // Verify socket creation and connection to remote server 
    if(p == NULL){
        fprintf(stderr, "client: failed to connect\n");
        return 2; 
    }

    // Print the remote server IPvX 
    char ipstr[INET6_ADDRSTRLEN];
    inet_ntop(p->ai_family, getSockAddrVx(p->ai_addr), ipstr, sizeof(ipstr));
    printf("client: connecting to server %s\n", ipstr);

    size_t bytes_received = 0;
    if((bytes_received = recv(sock_fd, buffer, MAXDATASIZE - 1, 0)) == -1){
        perror("client: recv()");
        exit(EXIT_FAILURE);
    }
    buffer[bytes_received] = '\0';

    printf("client: received '%s'\n", buffer);

    close(sock_fd);

    return 0; 
}

void print_usage(){
    printf("Usage: client [ARGUMENTS]\n");
    printf("Arguments:\n");
    printf("  -i, --server_hostname <IP or hostname>  Specify the server IP or hostname\n");
    printf("  -p, --server_port     <port_number>     Specify the server listener port number\n");
    printf("  -h, --help                              Show this help message\n");
}
