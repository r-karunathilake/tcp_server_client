/* 
    This is a simple TCP server written in the C programming language.
    The program is written and tested on Ubuntu. Furthermore, this 
    code is IP version-agnostic. 
            
        Ubuntu version:         22.04.3 LTS
        lippcap version:        1.10.1
        C/C++ compiler version: GCC 11.4.0
        Linux kernel version:   5.15.90.1-microsoft-standard-WSL2
        
    Author: Ravindu Karunathilake
    Date:   2024/03/22
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

/* This port number is used by kernel to match incoming packets to a certain
   process's socket descriptor. 

Note: All port numbers below 1024 are reserved, and any port above 
      and up to 65535 may be used here (provided the port is free). Well known
      ports-service bindings can be viewed at the IANA website.    
*/
#define PORT "666" // DOOM reference :) 
#define BACKLOG 10 // This is the queue length for incoming connections to this server

/* The internet socket (in Linux this a file descriptor used to communicate 
   with other programs) address structure. There are two types of internet 
   sockets: 'Stream Socket' and 'Datagram Socket'. Since this server needs
   reliable two-way communication, a stream socket with TCP will be used. 

   The socket address structure is as follows:

   struct addrinfo{
    int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, AI_ADDRCONFIG etc. 
    int              ai_family;    // AF_INET, AF_INET6, AD_UNSPEC (IP version-agnostic)
    int              ai_socktype;  // SOCK_STREAM, SOCK_DRAM, 0 (any type)
    int              ai_protocol;  // 0 (any protocol)
    size_t           ai_addrlen;   // size of 'ai_addr' in bytes 
    struct sockaddr  *ai_addr;     // struct sockaddr_in (IPv4) or sockaddr_in6 (IPv6)
    char             *ai_canonname // full canonical hostname 
    struct addrinfo  *ai_next;     // Linked list of 'addrinfo' structures 
   };

   Note: see man page of 'getaddrinfo' for more details. 

   struct sockaddr{
    unsigned short int  sa_family;   // address family, AF_XXX (2 bytes on my system)
    char                sa_data[14]; // 14 bytes for the protocol address
   };

   For portability concerns, typically the 'sockaddr' structure inside 
   'addrinfo' structure is a different structure that's typed cast to 
   'addrinfo' before used with the 'getaddrinfo()' function call. These 
   structures are 'sockaddr_in' and 'sockaddr_in6'.
*/

// Function prototypes
void sockAddrPrint(struct addrinfo *);
void *getSockAddrVx(struct sockaddr *); // Get socket address, IPv4 or IPv6
void sigChildHandler(int); 
void childProcess(int sock_fd, int connection_fd);

int main(int argc, char* argv[]){
    int sock_fd;                        // Server will listen on this fd 
    int new_con_fd;                     // New server connections on this fd
    struct addrinfo hints;              // Starting socket configuration 
    struct addrinfo *servinfo_res;      // Head of the linked list populated by 'getaddrinfo()' call
    struct sockaddr_storage con_addr;   // Incoming connection address information 

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;        // Use current host IP as part of the socket

    int status = -1; 
    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo_res)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1; 
    }

    printf("Server IP addresses are:\n\n");
    // Create and bind socket to the first available address 
    struct addrinfo *p;
    for(p = servinfo_res; p != NULL; p = p->ai_next){
        // Make a socket
        if((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
            perror("server: socket creation");
            continue;
        }

        // Print the socket address information to STDOUT
        sockAddrPrint(p);

        // Ensure socket is clearned from the kernel cleanly after disconnection
        int yes = 1;
        if(setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
            perror("server: socket level options");
            exit(1);
        }

        // Bind the socket to the port 
        if(bind(sock_fd, p->ai_addr, p->ai_addrlen) == -1){
            perror("server: socket bind");
            exit(1);
        }

        break; // Socket is now created and bound 
    }

    // Free the linked list populated by 'getaddrinfo()' call
    freeaddrinfo(servinfo_res); 

    // Verify socket creation and bind 
    if(p == NULL){
        fprintf(stderr, "server: failed to bind socket\n");
        exit(1);
    }

    // Server will listen to incoming connections 
    if(listen(sock_fd, BACKLOG) == -1){
        perror("server: listening");
        exit(1);
    }

    // Reap all the zombie processes created by the fork() system call
    struct sigaction sa; // Action to be taken when a signal arrives 
    sa.sa_handler = sigChildHandler; 
    // No additional signals will be blocked while this handler is executing
    sigemptyset(&sa.sa_mask); 
    // Restart the system calls if interrupted by another signal
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("server: sigaction");
        exit(1);
    }

    printf("server: listening for connections...\n");
    while(1){
        // Accept pending connection requests 
        socklen_t con_addr_size = sizeof(con_addr);
        if((new_con_fd = accept(sock_fd, (struct sockaddr *)&con_addr, &con_addr_size)) == -1){
            perror("server: accepting connection");
            continue; 
        }

        char ipstr[INET6_ADDRSTRLEN];
        inet_ntop(con_addr.ss_family, getSockAddrVx((struct sockaddr *)&con_addr), 
                  ipstr, sizeof(ipstr));
        printf("server: received connection from %s\n", ipstr);

        /* Fork a child process to handle this connection. The fork()
        system call return 0 to the child process and a positive value (PID)
        to the parent. The returned process ID is of type 'pid_t' defined in 
        the 'sys/types.h'. Additonally, the Linux kernel will supply a parent
        address space copy as needed to the child.*/

        if(!fork()){ // Execute if this is the child process  
            childProcess(sock_fd, new_con_fd); 
        }
        close(new_con_fd); // No longer needed by the parent process 
    }

    return 0;
}

void sockAddrPrint(struct addrinfo *pSockAddr){
    void *addr; 
    char *ipver; 

    // Get the pointer to the address 
    if(pSockAddr->ai_family == AF_INET){ // IPv4
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)pSockAddr->ai_addr;
        addr = &(ipv4->sin_addr);
        ipver = "IPv4"; 
    }
    else{ //IPv6
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)pSockAddr->ai_addr;
        addr = &(ipv6->sin6_addr);
        ipver = "IPv6";
    }

    // Print IP address to STDOUT 
    char ipstr[INET6_ADDRSTRLEN];
    inet_ntop(pSockAddr->ai_family, addr, ipstr, sizeof ipstr);
    printf("    %s: %s\n", ipver, ipstr);
}

void *getSockAddrVx(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){// IPv4
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    // IPv6
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

void sigChildHandler(int s){
    (void) s; 
    // Save the 'errno' status as 'waitpid()' might overwrite
    int saved_errono = errno;

    // Wait for all child processses to exit 
    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errono; 
}

void childProcess(int sock_fd, int con_fd){
    close(sock_fd); // Child process doesn't need the listener 
    char *msg = "Hello, from the other side!";
    size_t bytes_sent = 0; 
    if((bytes_sent = send(con_fd, msg, strlen(msg), 0)) == -1){
        perror("server: send"); 
    }
    close(con_fd);
    exit(0);
}
