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

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

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

int main(int argc, char* argv[]){
    int sock_fd;                   // server will listen on this fd 
    int new_con_fd;                // new server connections on this fd
    struct addrinfo hints;         // Starting socket configuration 
    struct addrinfo *servinfo_res; // head of the linked list populated by 'getaddrinfo()' call

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;   // Use current host IP as part of the socket

    int status = -1; 
    if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo_res)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1; 
    }

    printf("Server IP addresses are:\n\n");
    for(struct addrinfo *p = servinfo_res; p != NULL; p = p->ai_next){
        void *addr; 
        char *ipver; 

        // Get the pointer to the address 
        if(p->ai_family == AF_INET){ // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4"; 
        }
        else{ //IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // Print IP address to STDOUT 
        char ipstr[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf("    %s: %s\n", ipver, ipstr);
    }

    // free the linked list populated by 'getaddrinfo()' call
    freeaddrinfo(servinfo_res); 
    return 0;
}
