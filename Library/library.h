#ifndef LIBRARY_H
#define LIBRARY_H

#include "socketwrappers.h"

#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <iostream>
#include <fstream>

#define SERVER_PORT     7005
#define TRANSFER_PORT   70005
#define EXIT            0
#define GET             1
#define SEND            2

#define DATA		0
#define ACK		1
#define EOT		2

#define CLIENTIP 0
#define CLIENTPORT 1
#define EMULATORIP 2
#define EMULATORPORT1 3
#define EMULATORPORT2 4
#define SERVERIP 5
#define SERVERPORT 6

#define RECVTIMER 1 
#define SENDTIMER 10 
#define MAXTIMEOUT 10

#define PACKET_INT 32

using namespace std;

typedef struct
{
	int type;
	char filename[BUFLEN];	//isn't this too much?
}Cmd;

//extern "C" typedef struct
typedef struct
{
	//char Type[PACKET_INT];			//numeric code (ACK, DATA or EOT)
	int Type;
	//char SeqNum[PACKET_INT];
	int SeqNum;
	char Data[BUFLEN];
	//char WindowSize[PACKET_INT];			//number of packets being sent in first burst, size of window
	int WindowSize;
	//char AckNum[PACKET_INT];
	int AckNum;
}Packet;

typedef struct
{
	char Type[PACKET_INT];
	char SeqNum[PACKET_INT];
	char Data[BUFLEN];
	char WindowSize[PACKET_INT];
	char AckNum[PACKET_INT];
}PacketBuffer;

//command functions
Cmd CreateCmd(int type, char *filename);
Cmd RecvCmd(int sockfd);
bool SendCmd(int socket, Cmd cmd);
int RecvCmdNoBlock(int socket, Cmd *cmd);
int SendCmdNoBlock(int socket, Cmd *cmd);
int rRecvCmd(int socket, Cmd *cmd);
int rSendCmd(int socket, Cmd *cmd);

//Packet functions
Packet CreatePacket(int type, int SeqNum, char data[BUFLEN], int WindowSize, int AckNum);
int ReadPacket(int socket, Packet *packet);
int SendPacket(int socket, Packet *packet);
void PrintPacket(Packet packet,FILE *logs);
char* Packetize(Packet *packet);
Packet Unpacketize(char* buffer);


//transmitter and receiver functions
void RecvFile(int socket, char* filename, FILE *logs);
void SendFile(int socket, char *filename, FILE *logs);

//string/file manipulation functions
char *ParseString(std::string str);
bool isValidFile(char *cfilename);
void GetConfig(char *filename, std::string config[]);


#endif //LIBRARY_H
