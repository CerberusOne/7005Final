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

#define BUFLEN          1024
#define SERVER_PORT     7005
#define TRANSFER_PORT   70005
#define EXIT            0
#define GET             1
#define SEND            2

#define DATA		0
#define ACK		1
#define EOT		2

typedef struct 
{
	int type;
	char filename[BUFLEN];	//isn't this too much?
}Cmd;

typedef struct
{
	int Type;			//numeric code (ACK, DATA or EOT)
	int SeqNum;			
	char Data[BUFLEN];
	int WindowSize;			//number of packets being sent in first burst, size of window
	int AckNum;
}Packet;

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
//bool SendFile(char* filename, int socket);
//void RecvFile(int socket, char* filename);
Cmd RecvCmd(int sockfd);
Cmd CreateCmd(int type, char *filename);
Packet CreatePacket(int type, int SeqNum, char data[BUFLEN], int WindowSize, int AckNum);
void PrintPacket(Packet packet);
#endif //SOCKETWRAPPERS_H

