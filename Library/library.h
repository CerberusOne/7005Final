#ifndef LIBRARY_H
#define LIBRARY_H

#include "socketwrappers.h"

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

Cmd RecvCmd(int sockfd);
bool SendCmd(int socket, Cmd cmd); 
Cmd CreateCmd(int type, char *filename);
Packet CreatePacket(int type, int SeqNum, char data[BUFLEN], int WindowSize, int AckNum);
void PrintPacket(Packet packet);
void RecvFile(int socket, char* filename);
void SendFile(int socket, char *filename);


#endif //LIBRARY_H
