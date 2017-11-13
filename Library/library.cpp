/*------------------------------------------------------------------------------------------------
-- SOURCE FILE: library.cpp
--
-- PROGRAM:     COMP7005 - Final Project
--
-- FUNCTIONS:  
-- DATE:        Nov 12, 2017
--
-- DESIGNER:    Aing Ragunathan
--
-- PROGRAMMER:  Aing Ragunathan
--
-- NOTES:       Socket wrappers used for client and server communication in assignment 1
--------------------------------------------------------------------------------------------------*/
#include "library.h"

using namespace std;
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

/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   RecvCmd
--
-- DATE:       Oct 2, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  Cmd RecvCmd(int sockfd)
--
-- PARAMETER:  	int sockfd 		- file descriptor to connect with
--
-- RETURNS:    return a Cmd object containing a command type and filename
--
-- NOTES:      wrapper function for receiving a Cmd object
----------------------------------------------------------------------------------------------- */
Cmd RecvCmd(int sockfd) {
	Cmd cmd;
	int bytesRecv;

	if((bytesRecv = recv(sockfd, &cmd, sizeof(Cmd), 0)) == -1) {
		perror("RecvCmd Failed");
	} else if(bytesRecv == 0) {
		printf("Connection ended\n");
		cmd = CreateCmd(0, NULL);
	}

    printf("RecvCmd: %d %s\n", cmd.type, cmd.filename);

	return cmd;
}

/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   CreateCmd
--
-- DATE:       Oct 2, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  Cmd CreateCmd(int type, char *filename) 
--
-- PARAMETER:  	int type 		- the type of command to attach to the Cmd object
--				char *filename	- the filename to attach to the Cmd object
--
-- RETURNS:    return a Cmd object containing a command type and filename
--
-- NOTES:      wrapper function for creating a Cmd object
----------------------------------------------------------------------------------------------- */
Cmd CreateCmd(int type, char *filename) {
	Cmd cmd;

	cmd.type = type;
    strcpy(cmd.filename, filename);

	return cmd;	
}

/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   CreatePacket
--
-- DATE:       No 11, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  Packet CreatePacket(int type, int SeqNum, char *PayloadLen, int WindowSize, int AckNum) 
--
-- PARAMETER:  	
--
-- RETURNS:   
--
-- NOTES:    
----------------------------------------------------------------------------------------------- */
Packet CreatePacket(int type, int SeqNum, char data[BUFLEN], int WindowSize, int AckNum) {
	Packet packet;

	packet.Type = type;
	packet.SeqNum = SeqNum; 
    	strcpy(packet.Data, data);
	packet.WindowSize = WindowSize;
	packet.AckNum = AckNum;	

	return packet;	
}

//print content of the file
void PrintPacket(Packet packet) {
	printf("Type: Data\n");
        printf("SeqNum: %d\n", packet.SeqNum);
	printf("WinSize: %d\n", packet.WindowSize);
	printf("AckNum: %d\n", packet.AckNum);
	printf("Payload: %s", packet.Data);
}


bool SendCmd(int socket, Cmd cmd) {
    if(send(socket, &cmd, sizeof(cmd), 0) == -1) {
        perror("SendCmd Failed");
        return false;
    }

    return true;
}




