# TCP Server-Client Program in C

This repository contains a steam server-client written in the C programming language.

## Features

- Customize IP/hostname and port on the command line for client connection.
- Send simple string between server and client. 
- Telnet to the server 

## Prerequisites

Before compiling and running this program, following prerequisites must be met:
- Linux/Unix based OS
- C Compiler (e.g. Clang, GCC)
  
## Installation 
1. Clone this repository to your local machinine. 
 - SSH: `git@github.com:r-karunathilake/tcp_server_client.git`
 - HTTPS: `https://github.com/r-karunathilake/tcp_server_client.git`

2. Compile the the server and client programs using your C compiler of choice:
   - Manually: `clang -std=gnu17 server.c -o server` etc. 
   - Makefile: `make all` 
   - Makefile with debug information: `make BUILD=DEV`

## Usage
You can run the server as follows: `./server`. This will bind the server to the host IP on port `666` (can be changed by updating the macro in `server.c`).

> Note: require super user privileges to run the program.

The server client can be run by executing the following command: 
`./client [arguments]`

Replace `[arguments]` with the following:

- `-i <Server IP/Hostname>`: (Required) Specify the server hostname or IP to connect/
- `-p <Server Port Number>`: (Required) Specify the listen port for the server on the server host machine. 
- `-h`: (Optional) Display commandline usage information. 

## License
This packet capture program is open-source and available under the [MIT License](https://opensource.org/license/mit/).

## Author
    Ravindu Karunathilake 
    Email: karunath@ualberta.ca
    GitHub: https://github.com/r-karunathilake

## References 
1. [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/#select) by Brian Hall
