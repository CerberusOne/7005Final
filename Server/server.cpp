/*------------------------------------------------------------------------------------------------
-- SOURCE FILE: server.cpp
--
-- PROGRAM:     COMP7005 - Assignment 1 FTP
--
-- FUNCTIONS:   void SendFile(int socket, char *filename);
--				void RecvFile(int socket, char* filename);
--
-- DATE:        Oct 2, 2017
--
-- DESIGNER:    Aing Ragunathan
--
-- PROGRAMMER:  Aing Ragunathan
--
-- NOTES:       Server class methods for creating and using a client for the FTP program
--------------------------------------------------------------------------------------------------*/

#include <fstream>
#include "../A1/client.h"
#include "../A1/server.h"
#include "../A1/socketwrappers.h"


#define PATH "./Server_files/"

void SendFile(int socket, char *filename);
void RecvFile(int socket, char* filename);


using namespace std;

/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   Main
--
-- DATE:       Oct 2, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  main()
--
-- PARAMETER:  int argc, char *argv[]
--
-- RETURNS:    void
--
-- NOTES:      Constructor for a Server
----------------------------------------------------------------------------------------------- */
int main (int argc, char *argv[]) {
	Cmd cmd;			//command structure 
	char path[BUFLEN];	//path of files according to the root directory of the executable

	Server *commandConnection = new Server(7005);					//Server object for commands	
	Client *transferConnection = new Client(commandConnection->GetTransferIP(), 70005);//Client object for transfers
	
	do{ 
		cmd = commandConnection->WaitCommand();					//Wait for the client 
		printf("Type: %d\n",cmd.type);
		printf("Filename: %s\n",cmd.filename);

		strcpy(path, PATH);
		strcat(path, cmd.filename);

		if(cmd.type == SEND) {
			RecvFile(transferConnection->GetSocket(), path);
		} else if (cmd.type == GET) {
			SendFile(transferConnection->GetSocket(), path);
		}

	} while (cmd.type != EXIT);

//	fclose(transferConnection->GetSocket());
//	fclose(commandConnection->GetSocket());

	return 0;
}

/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   SendFile
--
-- DATE:       Oct 2, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  void SendFile(int socket, char *filename) 
--
-- PARAMETER:  	int socket 		- socket to send the data into
--				char *filename	- file to send through the socket
--
-- RETURNS:    void
--
-- NOTES:      Sends an entire file through the socket
----------------------------------------------------------------------------------------------- */
void SendFile(int socket, char *filename) {
	FILE *file;
	char buffer[BUFLEN];
	int bytesRead, bytesSent, seqNum;	
	Packet packet;

	if((file = fopen(filename, "rb")) == NULL) {
		perror("file doesn't exist\n");
		return;
	}

	memset(buffer, '\0', BUFLEN);
	memset(packet.Data, '\0', BUFLEN);

	while((bytesRead = fread(buffer, 1, sizeof(buffer), file)) != 0) {
		//calculate the sequence number for the next packet (move to end of loop)
		seqNum = bytesRead + packet.SeqNum;

		//Send the DATA packet
		packet = CreatePacket(DATA, seqNum, buffer, 0, 0);
		if((bytesSent = write(socket, &packet, sizeof(packet))) == -1) {
			perror("Error writing to socket DATA: ");
			return;
		} 

		memset(buffer, '\0', BUFLEN);
		
		//send the EOT packet: End of transmission
		if(bytesRead < BUFLEN) {
			packet = CreatePacket(EOT, seqNum, buffer, 0, 0);	
			if((bytesSent = write(socket, &packet, sizeof(packet))) == -1) {
				perror("Error writing to socket EOT: ");
				return;
			}
		}
	}
	
	fclose(file);
}


/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   RecvFile
--
-- DATE:       Oct 2, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  void RecvFile(int socket, char* filename)
--
-- PARAMETER:  	int socket 		- socket to send the data into
--				char *filename	- file to send through the socket
--
-- RETURNS:    void
--
-- NOTES:      Receives an entire file through the socket
----------------------------------------------------------------------------------------------- */
void RecvFile(int socket, char* filename) {
	FILE *file;
	char buffer[BUFLEN];
	int bytesRecv, writeCount = 0;
	Packet packet;

	//open file to write in binary
	if((file = fopen(filename, "wb")) == NULL) {
		printf("file failed to open: %s\n", filename);
		return;
	}	

	truncate(filename, 0);

	while(1) {
		bzero(buffer, sizeof(buffer));

		//receive the packet
		if((bytesRecv = read(socket, &packet, sizeof(packet))) < 0) {
			perror("Error receiving from socket");
			return;
		} else {
			//check the packet type and treat accordingly
			if(packet.Type == DATA) {
				PrintPacket(packet);	//print content of file
			
				if((writeCount = fwrite(packet.Data, 1, strlen(packet.Data), file)) < 0) {
					perror("Write failed");
					return;
				}

			} else if(packet.Type == EOT) {
				printf("Type: EOT\n");
				fclose(file);
				return;
			}
		}
	}
}




