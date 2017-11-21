#ifndef SOCKETWRAPPERS_H
#define SOCKETWRAPPERS_H

#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFLEN		1024

int Socket(int family, int type, int protocol);
void SetReuse (int socket);
int Bind(int socket, struct sockaddr_in *addr);
int Listen(int socket, int size);
int Accept(int socket, struct sockaddr *addr, socklen_t *addrlen);
void ConfigServerSocket(struct sockaddr_in *servaddr, int port);
void ConfigClientSocket(struct sockaddr_in *servaddr, const char* ip, int port);
bool Connect(int sockfd, struct sockaddr_in sockaddr);
bool SendMsg(int sockfd, char* buffer);
int RecvMsg(int sockfd, char* buffer);
void SetNonBlocking(int socket);

#endif //SOCKETWRAPPERS_H

