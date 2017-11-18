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

#include "../Library/socketwrappers.h"
#include "../Library/library.h"
#include "../Library/client.h"
#include "../Library/server.h"

#define PATH "./Server_files/"

//void SendFile(int socket, char *filename);
//void RecvFile(int socket, char* filename);


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
void SetBlocking(int socket);
Packet RecvPacket(int socket, char* filename);
void WritePacket(Packet packet);
Packet SendACK(int socket, Packet packet,int packettype,int SEQ);
bool isValidFile(char *cfilename);
void Timeout();

int main (int argc, char *argv[]) {
	Packet packet;
	int expectedSEQ;
	Cmd cmd;			//command structure
	char path[BUFLEN];	//path of files according to the root directory of the executable

	Server *commandConnection = new Server(7005);					//Server object for commands
	SetBlocking(commandConnection->GetSocket());
	Client *transferConnection = new Client(commandConnection->GetTransferIP(), 70005);//Client object for transfers
	SetBlocking(transferConnection->GetSocket());

	do{
		//cmd = commandConnection->WaitCommand();					//Wait for the client
		cmd = RecvCmd(commandConnection->GetSocket());
		printf("Type: %d\n",cmd.type);
		printf("Filename: %s\n",cmd.filename);

		strcpy(path, PATH);
		strcat(path, cmd.filename);

		if(cmd.type == SEND) {
			RecvFile(transferConnection->GetSocket(), path);
			if (packet.SeqNum == expectedSEQ){
				packet = RecvPacket(transferConnection->GetSocket(), path);
				if (packet.type == DATA && packet.SeqNum == expectedSEQ){
						WritePacket(packet);
						SendACK(transferConnection->GetSocket(),ACK,expectedSEQ);
						expectedSEQ += BUFLEN;
				} else if (packet.type == EOT && packet.SeqNum == expectedSEQ){
					WritePacket(packet);
					SendACK(transferConnection->GetSocket(),EOT,expectedSEQ);
					expectedSEQ += BUFLEN;
				}
			}
		} else if (cmd.type == GET) {
			SendFile(transferConnection->GetSocket(), path);
		}

	} while (cmd.type != EXIT);

	close(transferConnection->GetSocket());
	close(commandConnection->GetSocket());

	return 0;
}

void SetBlocking (int socket){
	rc = fcntl(socket,F_SETFL,O_NONBLOCK);
	if (rc < 0){
		 perror("Error setting socket to non-blocking");
		exit(3);
	}
}

Packet RecvPacket(int socket, char* filename) {
	int bytesRecv, writeCount = 0;
	Packet packet;

	//receive the packet
	while((bytesRecv = read(socket, &packet, sizeof(packet))) > 0) {
			//check the packet type and treat accordingly
			if(packet.Type == DATA) {
				printf("Type: DATA\n");
				PrintPacket(packet);	//print content of file
			} else if(packet.Type == EOT) {
				printf("Type: EOT\n");
			}
		}
		return packet;
	}

	void WritePacket(Packet packet){
		FILE *file;
		int writeCount = 0;
		Packet packet;

		//open file to write in binary
		if((file = fopen(filename, "wb")) == NULL) {
			printf("file failed to open: %s\n", filename);
			return;
		}

		PrintPacket(packet);	//print content of file

		if((writeCount = fwrite(packet.Data, 1, strlen(packet.Data), file)) < 0) {
			perror("Write failed");
			return;
		}

	}

	Packet SendACK(int socket, Packet packet,int packettype,int SEQ){
			int bytesRead, bytesSent;

				//Send the DATA packet
				if((bytesSent = write(socket, &packet, sizeof(packet))) == -1) {
					perror("Error writing to socket DATA: ");
					return;
				}
			return packet;
	}

	bool isValidFile(char *cfilename) {
		FILE *file;

		if((file = fopen(cfilename, "rb")) == NULL) {
			printf("file doesn't exist\n");
			return false;
		}

		fclose(file);
		return true;
	}
