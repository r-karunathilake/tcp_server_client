#ifndef UTILITY_H
#define UTILITY_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>

// Get socket address, IPv4 or IPv6
void *getSockAddrVx(struct sockaddr *sa){
    if(sa->sa_family == AF_INET){// IPv4
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }
    // IPv6
    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
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

#endif
