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
--		Expects to use a non-blocking socket
----------------------------------------------------------------------------------------------- */
void SendFile(int socket, char *filename) {
	FILE *file;
	char buffer[BUFLEN];
	int bytesRead, bytesSent;
	Packet packet;

	int seqNum = 0;
	int ackNum = 0;
	int windowSize = 4 * BUFLEN;	//bytes in transmission
	int base = 0;			//base +1 is the next expected ACK
	int nextSeq = base;

	//open file
	if((file = fopen(filename, "rb")) == NULL) {
		perror("file doesn't exist\n");
		return;
	}

	//reset buffers
	memset(buffer, '\0', BUFLEN);
	memset(packet.Data, '\0', BUFLEN);

	int passed;
	clock_t start = 0;
	bool send = false;

	while(1) {
		//check data socket for new ACK in NON-BLOCKING
		if((bytesRead = read(socket, &packet, sizeof(packet))) != -1) {
			printf("received packet\n");

			//check packet type for EOT or ACK
			if(packet.Type == EOT) {
				printf("EOT found, ending transmission\n");
				fclose(file);
				return;
			}
			else if(packet.Type == ACK) {
				printf("ACK found: %d\n", packet.AckNum);

				//check if it is the next expected packet, otherwise discard
				if(packet.AckNum == base + (int)sizeof(packet.Data)) {
					//increment base of window (slide window)
					base += sizeof(packet.Data);

					//if the window is empty
					if(base == nextSeq) {
						//stop to timer
						start = 0;
						printf("Starting timer\n");

					} else {
						//restart the timer
						start = clock();
						printf("Restarting timer\n");
					}
				} else {
					//discard packet
					printf("discarding packet, recv: %d\texpected: %d \n", packet.AckNum, base + (int)sizeof(packet.Data));
					//move window's base to next expected byte for the receiver
					base = packet.AckNum;
				//	nextSeq = base;
					seqNum = base;
					send = false;
					if(fseek(file, base, SEEK_SET) < 0) {
						perror("duplicate ack fseek");	
					}
				}
			}
		} else if (bytesRead == -1) {
			if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
				perror("ERROR not EAGAIN or EWOULDBLOCK");
				return;
			}
		}

		//check if there is a timeout
		passed = (clock() - start)/CLOCKS_PER_SEC;
		if(passed >= 10){
			printf("timeout, base: %d\n", base);
			//set nextSeq to base
			nextSeq = base;
			seqNum = base;
			send = false;
			//seek file back to bytesRead - base
			if(fseek(file, base, SEEK_SET) < 0) {
				perror("fseek");
			}
		}

		//only send next packet if window isn't full
		if(nextSeq < base + windowSize) {
			//check if we are ready to send and if a packet hasn't been created
			if(!send) {
				//read file
				if((bytesRead = fread(buffer, sizeof(char), sizeof(buffer), file)) != -1) {

					//check for EOT and send packet
					if(bytesRead < (int)sizeof(buffer)) {
						packet = CreatePacket(EOT, seqNum, buffer, 0, 0);
					} else {
						packet = CreatePacket(DATA, seqNum, buffer, windowSize, ackNum);
					}

					if(bytesRead > 0) {
						send = true; //packet ready to send
						printf("Ready to send, seq %d\n", seqNum);
					}
				} else {
					perror("Reading file: ");
					exit(1);
				}


			}

			if(send) {
				if((bytesSent = write(socket, &packet, sizeof(packet))) != -1) {
					printf("bytes sent: %d\n", bytesSent);
					seqNum += bytesRead;		//calculate next seq number
					memset(buffer, '\0', BUFLEN);	//reset buffer

					if(base == nextSeq) {
						//start timer
						start = clock();
						printf("Starting timer, base: %d\n", base);
					}

					nextSeq += bytesRead;	//update next sequence
					send = false;	//not ready to send another packet, packet used

				} else if (bytesSent == -1) {
					//perror("Send File: Error writing to socket DATA");

					if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
						perror("ERROR not EAGAIN or EWOULDBLOCK");
						return;
					}
				}
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
	int bytesSent, expectedSEQ=0, bytesRecv, writeCount = 0;
	Packet packet;

	//open file to write in binary
	if((file = fopen(filename, "wb")) == NULL) {
		printf("file failed to open: %s\n", filename);
		return;
	}
	truncate(filename, 0);

	while(1) {
		memset(&packet, 0, sizeof(packet));
		//receive the packet
		if((bytesRecv = read(socket, &packet, sizeof(packet))) == -1) {
			if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
				perror("ERROR not EAGAIN or EWOULDBLOCK");
				return;
			}
		} else {
			//check the packet type and treat accordingly
			if(packet.Type == DATA && packet.SeqNum == expectedSEQ) {
				//write file
				if((writeCount = fwrite(packet.Data, 1, strlen(packet.Data), file)) <0){
					perror("Write failed");
					return;
				}
				PrintPacket(packet);	//print content of file
				
				//update expectedSEQ
				expectedSEQ+=sizeof(packet.Data);
				memset (&packet, 0, sizeof(packet));
				
				//create ACK packet
				packet.Type = ACK;
				packet.AckNum = expectedSEQ;
				if((bytesSent = write(socket, &packet, sizeof(packet))) == -1) {
					if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
						perror("ERROR not EAGAIN or EWOULDBLOCK");
						return;
					}
				}

			}else if(packet.Type == DATA && packet.SeqNum != expectedSEQ){
				printf("Duplicate packet discarded\n");
				printf("SeqNum: %d\n",packet.SeqNum);
				
				memset (&packet, 0, sizeof(packet));
				
				//create ACK packet
				packet.Type = ACK;
				packet.AckNum = expectedSEQ;
				if((bytesSent = write(socket, &packet, sizeof(packet))) == -1) {
					if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
						perror("ERROR not EAGAIN or EWOULDBLOCK");
						return;
					}
				}

			} else if(packet.Type == EOT && packet.SeqNum == expectedSEQ) {
				printf("\nRecv Data\n");
				printf("Type: EOT\n");
				printf("SeqNum: %d\n",packet.SeqNum);


				if((writeCount = fwrite(packet.Data, 1, strlen(packet.Data), file)) < 0) {
					perror("RecvFile write failed");
					return;
				}

				//reset the EOT packet
				memset (&packet, 0, sizeof(packet));
				packet.Type = EOT;
				packet.AckNum = expectedSEQ;

				//send back the EOT packet for confirmation
				if((bytesSent = write(socket, &packet, sizeof(packet))) == -1) {
					if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
						perror("ERROR not EAGAIN or EWOULDBLOCK");
						return;
					}
				}

				//update expectedSEQ
				expectedSEQ+=BUFLEN;
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
	printf("Type: %d\n", packet.Type);
        printf("SeqNum: %d\n", packet.SeqNum);
	printf("WinSize: %d\n", packet.WindowSize);
	printf("AckNum: %d\n", packet.AckNum);
	//printf("Payload: %s", packet.Data);
}


bool SendCmd(int socket, Cmd cmd) {
    if(send(socket, &cmd, sizeof(cmd), 0) == -1) {
        perror("SendCmd Failed");
        return false;
    }

    return true;
}

char *ParseString(string str){
    	char *cstr;

    	cstr = new char[str.length() + 1];
    	strcpy(cstr, str.c_str());

    	return cstr;
}

void GetConfig(char *filename, std::string config[]){
	ifstream file;
     	file.open(filename);
     	//check if file exists
        if (!file){
        	perror("Files does not exist");
       	} else {
        	int i;
                for(i=0; i <=SERVERPORT; i++){
                	getline(file, config[i]);
                    	//cout << config[i] << endl;
                }
        }
}


/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   isValidFile
--
-- DATE:       Oct 2, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  bool isValidFile(char *cfilename)
--
-- PARAMETER:  	char *cfilename 	- file to be checked
--
-- RETURNS:    true if the file exists
--
-- NOTES:      Checks if a file exists
----------------------------------------------------------------------------------------------- */
bool isValidFile(char *cfilename) {
	FILE *file;

	if((file = fopen(cfilename, "rb")) == NULL) {
		printf("file doesn't exist\n");
		return false;
	}

	fclose(file);
	return true;
}
