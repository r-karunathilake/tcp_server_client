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

#include "utility.h" 

#define PORT "3456"
#define MAXDATASIZE 100 // Maximum number of bytes to be received 

// Function prototypes 


int main(int argc, char *argv[]){
    int sock_fd;
    char buffer[MAXDATASIZE]; 
    struct addrinfo hints; 
    struct addrinfo *clientinfo_res; 

    // Check correct number of arguments provided 
    if (argc != 2){
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;

    int status = -1;
    if((status = getaddrinfo(NULL, PORT, &hints, &clientinfo_res)) != 0){
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
        exit(1);
    }
    buffer[bytes_received] = '\0';

    printf("client: received '%s'\n", buffer);

    close(sock_fd);

    return 0; 
}
