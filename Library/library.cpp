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
void SendFile(int socket, char *filename, FILE *logs) {
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
	int timeoutCounter = 0;

	while(1) {
		//check data socket for new ACK in NON-BLOCKING
		//if((bytesRead = read(socket, &packet, sizeof(packet))) != -1) {
		//if((bytesRead = recv(socket, &packet, sizeof(packet), MSG_WAITALL)) != -1) {
		if((bytesRead = ReadPacket(socket, &packet)) != -1) {
			printf("received packet: %d\n", bytesRead);
			fprintf(logs,"received packet: %d\n", bytesRead);

			//check packet type for EOT or ACK
			if(packet.Type == EOT) {
				printf("EOT found, ending transmission\n");
				fprintf(logs,"EOT found, ending transmission\n");
				fclose(file);
				return;
			}
			else if(packet.Type == ACK) {
				printf("ACK found: %d\n", packet.AckNum);
				fprintf(logs,"ACK found: %d\n", packet.AckNum);
				base = packet.AckNum;
				timeoutCounter = 0;
				if(base == nextSeq) {
					start = 0;
					printf("Stopping timer\n");
					fprintf(logs,"Starting timer\n");
				} else {
					start = clock();
					printf("Starting timer\n");
					fprintf(logs,"Starting timer\n");
				}
			}
		} else if (bytesRead == -1) {
			if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
				perror("ERROR not EAGAIN or EWOULDBLOCK");
				fprintf(logs,"ERROR not EAGAIN or EWOULDBLOCK");
				return;
			}
		}

		//check if there is a timeout
		passed = (clock() - start)/CLOCKS_PER_SEC;
		if(passed >= SENDTIMER){
			timeoutCounter++;
			printf("timeout counter: %d\n", timeoutCounter);

			if(timeoutCounter == 10) {
				fclose(logs);
				exit(1);
			}

			printf("nextSeq: %d\n", nextSeq);
			fprintf(logs,"nextSeq: %d\n", nextSeq);
			printf("timeout, base: %d\n", base);
			fprintf(logs,"timeout, base: %d\n", base);
			printf("timeoutCounter: %d\n", timeoutCounter);
			fprintf(logs,"timeoutCounter: %d\n", timeoutCounter);

			//set nextSeq to base
			nextSeq = base;
			seqNum = base;
			send = false;
			//seek file back to bytesRead - base
			if(fseek(file, base, SEEK_SET) < 0) {
				perror("fseek");
			}

			//printf("\n");
		}


		//only send next packet if window isn't full
		if(nextSeq < base + windowSize) {
			//check if we are ready to send and if a packet hasn't been created
			if(!send) {
				//read file
				if((bytesRead = fread(buffer, sizeof(char), sizeof(buffer), file)) != -1) {
					//check for EOT and send packet
					if(bytesRead < (int)sizeof(buffer)) {
						packet = CreatePacket(EOT, seqNum, buffer, windowSize,ackNum);
					} else {
						packet = CreatePacket(DATA, seqNum, buffer, windowSize, ackNum);
					}

					if(bytesRead > 0) {
						send = true; //packet ready to send
						printf("Ready to send, seq %d\n", seqNum);
						fprintf(logs,"Ready to send, seq %d\n", seqNum);
					}
				} else {
					perror("Reading file: ");
					fprintf(logs,"Reading file: ");
					exit(1);
				}


			}

			if(send) {
				//if((bytesSent = write(socket, &packet, sizeof(packet))) != -1) {
				if((bytesSent = SendPacket(socket, &packet)) != -1) {
				//if((bytesSent = write(socket, &buffer, sizeof(buffer))) != -1) {
					printf("bytes sent: %d\n", bytesSent);
					fprintf(logs,"bytes sent: %d\n", bytesSent);
					seqNum += bytesRead;		//calculate next seq number
					memset(buffer, '\0', BUFLEN);	//reset buffer

					if(base == nextSeq) {
						//start timer
						start = clock();
						printf("Starting timer, base: %d\n", base);
						fprintf(logs,"Starting timer, base: %d\n", base);
					}

					nextSeq += bytesRead;	//update next sequence
					send = false;	//not ready to send another packet, packet used

				} else if (bytesSent == -1) {
					//perror("Send File: Error writing to socket DATA");

					if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
						perror("ERROR not EAGAIN or EWOULDBLOCK");
						fprintf(logs,"ERROR not EAGAIN or EWOULDBLOCK");
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
void RecvFile(int socket, char* filename, FILE *logs) {
	//ACK timer
	int timer = 0;
	clock_t start = 0;
	bool newAck = false;

	FILE *file;
	int bytesSent, expectedSEQ=0, writeCount = 0;
	Packet packet;
	int discardCounter = 0;

	int bytesRead = 0;

	//open file to write in binary
	if((file = fopen(filename, "wb")) == NULL) {
		printf("file failed to open: %s\n", filename);
		fprintf(logs,"file doesn't exist\n");
		return;
	}

	while(1) {
		//check if a packet has been received

		timer = (clock()-start)/CLOCKS_PER_SEC;
		//if the timer has ran out
		if(timer >= RECVTIMER) {
			if(newAck) {
				printf("Timer expired, sending ACK\n");
				newAck = false;
				start = 0;

				//send the cumulative ACK 
				memset(&packet, 0, sizeof(packet));
				packet.Type = ACK;
				packet.AckNum = expectedSEQ;
				
			
				//if((bytesSent = write(socket, &packet, sizeof(packet))) == -1) {
				if((bytesSent = SendPacket(socket, &packet)) == -1) {
					if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
						perror("ERROR not EAGAIN or EWOULDBLOCK");
						fprintf(logs,"ERROR not EAGAIN or EWOULDBLOCK");
						return;
					}
				}
			}
		}
		
		//receive the packet
		//if((bytesRead = read(socket, &packet, sizeof(packet))) == -1) {
		//if((bytesRead = recv(socket, &packet, sizeof(packet), 0)) == -1) {
		if((bytesRead = ReadPacket(socket, &packet)) == -1) {
			if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
				perror("ERROR not EAGAIN or EWOULDBLOCK");
				fprintf(logs,"ERROR not EAGAIN or EWOULDBLOCK");
				return;
			}
		} else {
			printf("bytesRead: %d\n", bytesRead);

			if(!newAck) {
				newAck = true;
				start = clock();
			}

			//check the packet type and treat accordingly
			if(packet.Type == DATA && packet.SeqNum == expectedSEQ) {


				//write file
				if((writeCount = fwrite(packet.Data, 1, strlen(packet.Data), file)) <0){
					perror("Write failed");
					fprintf(logs,"Write failed");
					return;
				}

				PrintPacket(packet,logs);	//print content of file
				//update expectedSEQ
				expectedSEQ+=sizeof(packet.Data);
				memset (&packet, 0, sizeof(packet));
				discardCounter = 0;
			
			} else if(packet.Type == DATA && packet.SeqNum != expectedSEQ){
				discardCounter++;

				printf("Wrong SEQ found, discarding %d packet\n", discardCounter);
				fprintf(logs,"Wrong SEQ found, discarding packet\n");
				printf("\tSEQ: %d\n",packet.SeqNum);
				fprintf(logs, "\tSEQ: %d\n",packet.SeqNum);
				printf("\tExpected SEQ: %d\n", expectedSEQ);
				fprintf(logs,"\tExpected SEQ: %d\n", expectedSEQ);
				
				//allows the logging file to be saved if things go to shit
				if(discardCounter == 10) {
					printf("10 duplicates found, closing\n");
					fclose(logs);
					exit(1);
				}

				memset (&packet, 0, sizeof(packet));

			} else if(packet.Type == EOT && packet.SeqNum == expectedSEQ) {
				printf("\nRecv Data\n");
				fprintf(logs,"\nRecv Data\n");
				printf("Type: EOT\n");
				fprintf(logs,"Type: EOT\n");
				printf("SeqNum: %d\n",packet.SeqNum);
				fprintf(logs,"SeqNum: %d\n",packet.SeqNum);

				if((writeCount = fwrite(packet.Data, 1, strlen(packet.Data), file)) < 0) {
					perror("RecvFile write failed");
					fprintf(logs,"RecvFile write failed");
					return;
				}

				//update expectedSEQ
				expectedSEQ+=BUFLEN;
				//reset the EOT packet
				memset (&packet, 0, sizeof(packet));
				packet.Type = EOT;
				packet.AckNum = expectedSEQ;

				//send back the EOT packet for confirmation
				//if((bytesSent = write(socket, &packet, sizeof(packet))) == -1) {
				if((bytesSent = SendPacket(socket, &packet)) == -1) {
					if((errno != EAGAIN) ||(errno != EWOULDBLOCK)){
						perror("ERROR not EAGAIN or EWOULDBLOCK");
						fprintf(logs,"ERROR not EAGAIN or EWOULDBLOCK");
						return;
					}
				}
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
Packet CreatePacket(int type, int seqNum, char data[BUFLEN], int windowSize, int ackNum) {
	Packet packet;
/*
	char typeStr[PACKET_INT];
	char seqNumStr[PACKET_INT];
	char windowSizeStr[PACKET_INT];
	char ackNumStr[PACKET_INT];

	sprintf(typeStr, "%d", type);
	sprintf(seqNumStr, "%d", seqNum);
	sprintf(windowSizeStr, "%d", windowSize);
	sprintf(ackNumStr, "%d", ackNum);

	imemcpy(packet.Type, typeStr, sizeof(Packet.Type));
	memcpy(packet.SeqNum, seqNumStr, sizeof(Packet.SeqNum));
	memcpy(packet.WindowSize, windowSizeStr, sizeof(Packet.WindowSize);
	memcpy(packet.AckNum, ackNumStr, sizeof(Packet.AckNum);
*/
	
	
	packet.Type = type;
	packet.SeqNum = seqNum;
    	strcpy(packet.Data, data);
	packet.WindowSize = windowSize;
	packet.AckNum = ackNum;

	return packet;
}

//print content of the file
void PrintPacket(Packet packet,FILE *logs) {
	printf("Type: Data\n");
	fprintf(logs, "Type: Data\n");
    printf("SeqNum: %d\n", packet.SeqNum);
    fprintf(logs,"SeqNum: %d\n", packet.SeqNum);
	printf("WinSize: %d\n", packet.WindowSize);
	fprintf(logs,"WinSize: %d\n", packet.WindowSize);
	printf("AckNum: %d\n", packet.AckNum);
    fprintf(logs,"AckNum: %d\n", packet.AckNum);
	//printf("Payload: %s", packet.Data);
}


bool SendCmd(int socket, Cmd cmd) {
    	if(send(socket, &cmd, sizeof(cmd), 0) == -1) {
        	perror("SendCmd Failed");
        	return false;
    	}

    	return true;
}

int RecvCmdNoBlock(int socket, Cmd *cmd) {
	int bytesRead = 0;

//	if((bytesRead = read(socket, cmd, sizeof(Cmd))) != -1) {
	if((bytesRead = recv(socket, cmd, sizeof(Cmd), 0)) != -1) {
		printf("received command\n");
		return bytesRead;

	} else if(bytesRead == -1) {
		if((errno != EAGAIN) || (errno != EWOULDBLOCK)) {
			perror("ERROR not EAGAIN or EWOULDBLOCK");
			return -1;
		}
	}

	return -1;
}

int SendCmdNoBlock(int socket, Cmd *cmd) {
	int bytesSent = 0;
	
	if((bytesSent = write(socket, cmd, sizeof(Cmd))) != -1) {
		printf("bytesSent: %d\n", bytesSent);
		return bytesSent;	

	} else if (bytesSent == -1) {
		if((errno != EAGAIN) || (errno != EWOULDBLOCK)) {
			perror("ERROR not EAGAIN or EWOULDBLOCK");
			return -1;
		}
	}

	return -1;
}

//reliabily sends a file using ACKs
int rSendCmd(int socket, Cmd *cmd) {
	int passed = 0;
	clock_t start = 0;
	bool send = true;
	int timeoutCounter = 0;	//returns false when reaches MAX timeouts
	int bytesSent = 0;
	int bytesRead = 0;
	Cmd cmdRecv = {0};

	while(1) {
		if((bytesRead = RecvCmdNoBlock(socket, &cmdRecv)) != -1) {
			printf("rSendCmd: received ACK\n");
			return bytesRead;
		}

		passed = (clock() - start)/CLOCKS_PER_SEC;
		if(passed >= 10) {
			timeoutCounter++;
			send = true;
			printf("rSendCmd: timeout counter: %d\n", timeoutCounter);

			if(timeoutCounter >= MAXTIMEOUT) {
				printf("max timeout counter reached\n");
				return -1;
			}
		}
		
		if(send) {
			send = false;
	
			if((bytesSent = SendCmdNoBlock(socket, cmd)) != -1) {
				printf("rSendCmd: sent %d bytes\n", bytesSent);
			}
		}
	}

	return -1;
}

//recv and ACK the command before leaving function
int rRecvCmd(int socket, Cmd *cmd) {
	int passed = 0;
	clock_t start = 0;
	int sendCmd = 0;
	int timeoutCounter = 0;
	int bytesRead = 0;
	int cmdReceived = 0;
	int bytesSent = 0;

	while(1) {
		if((bytesRead = RecvCmdNoBlock(socket, cmd)) != -1) {
			printf("rRecvCmd: received Cmd\n");
			printf("rRecvCmd received: %d bytes\n", bytesRead);
			sendCmd = 1;
			cmdReceived = 1;	//in case send fails
		}

		passed = (clock() - start)/CLOCKS_PER_SEC;
		if(passed >= 100) {
			printf("rRecvCmd: timeout counter: %d\n", timeoutCounter);	
			timeoutCounter++;

			if(cmdReceived == 1)
				sendCmd = 1;	//in case send fails
			
			if(timeoutCounter >= MAXTIMEOUT) {
				printf("max timeout counter reached\n");
				return -1;
			}
		}

		//after cmd has been received
		if(sendCmd == 1) {
			sendCmd = 0;
			
			//send ACK 
			if((bytesSent = SendCmdNoBlock(socket, cmd)) != -1) {
				printf("rRecvCmd: sent %d bytes\n", bytesSent);
				return bytesRead;
			}
		}
	}
}

int ReadPacket(int socket, Packet *packet) {
	int bytesRead = 0;
	int result = 0;
	char buffer[sizeof(PacketBuffer)];
	char packetSize[4];
	long packetSizeLong = 0;
	//char space;
//what happens if one of the packets are dropped

/*
	//check if there's data in the socket, get the packetsize if there is
	if((result = recv(socket, packetSize, sizeof(packetSize), 0)) > 0) {
		//find the packet size
		packetSizeLong = strtol(packetSize, nullptr, 10);

		printf("ReadPacket: packetSize: %s\n", packetSize);
		printf("ReadPacket: packetSizeLong: %lu\n", packetSizeLong);
		printf("ReadPacket: bytesRead = %d\n", result);
		
		//recv(socket, &space, 1, 0);	//discard the space after the packet size			
*/		result = 0;
		int n = 0;

		//while(n < packetSizeLong) {
		while(n < sizeof(PacketBuffer)) {
			if((result = recv(socket, buffer + n, sizeof(PacketBuffer) - n, 0)) > 0) {

				n += result;	
				printf("ReadPacket: buffer: %s\n", buffer);
				printf("ReadPacket: bytesRead: %d\n", result);
				printf("ReadPacket: accumulation: %d\n\n", n);
			} else {
				return -1;
			}
		}

		printf("Buffer: %s\n",buffer);		
/*	} else {
		return -1;
	}
*/
	printf("Recv complete: %s\n", buffer);

	Unpacketize(buffer, packet);	

	printf("Receive Packet results:\n");
	printf("Packet.Type: %d\n", packet->Type);
	printf("Packet.SeqNum: %d\n", packet->SeqNum);
	printf("Packet.Data: %s\n", packet->Data);
	printf("Packet.WindowSize: %d\n", packet->WindowSize);
	printf("Packet.AckNum: %d\n", packet->AckNum);

	return bytesRead;
}

int SendPacket(int socket, Packet *packet) {
	int bytesSent = 0;
	char buffer[sizeof(PacketBuffer)] = {0};
	char sendBuf[sizeof(PacketBuffer) + sizeof(int)] = {0};
	char packetSize[4];
	int packetSizeInt = 0;

	
	Packetize(packet, buffer);	//convert packet to char array
	
	//packetSizeInt = sizeof(buffer) + sizeof(packetSize);	//get the total bytes to send
	//packetSizeInt = strlen(buffer) + 4;	//get the total bytes to send
	//memset(packetSize, 0, sizeof(packetSize));
	//snprintf(packetSize, sizeof(packetSize), "%d", packetSizeInt);	//convert total to string
	//snprintf(sendBuf, sizeof(PacketBuffer), "%s%s", packetSize, buffer);	//make send array
	
	//printf("SendPacket: sendBuf: %s\n", sendBuf);	
	printf("SendPacket: size of buffer: %d\n", (int)sizeof(buffer));
	printf("SendPacket: buffer: %s\n", buffer);

	//if((bytesSent = send(socket, sendBuf, sizeof(sendBuf), 0)) != -1) {
	if((bytesSent = send(socket, buffer, sizeof(buffer), 0)) != -1) {
		printf("BytesSent: %d\n", bytesSent);
		return bytesSent;
	} 		
	
	printf("SendPacket: failed to send: %d\n", bytesSent);

	return -1;
}

//buffer must be char[sizeof(PacketBuffer)]
void Packetize(Packet *packet, char buffer[]) {
	PacketBuffer packetBuffer =  {0};

	//copy content from Packet to PacketBuffer and change ints to char[]
	snprintf(packetBuffer.Type, sizeof(packetBuffer.Type), "%d", packet->Type);
	snprintf(packetBuffer.SeqNum, sizeof(packetBuffer.SeqNum), "%d", packet->SeqNum);
	snprintf(packetBuffer.Data, sizeof(packetBuffer.Data), "%s", packet->Data);
	snprintf(packetBuffer.WindowSize, sizeof(packetBuffer.WindowSize), "%d", packet->WindowSize);
	snprintf(packetBuffer.AckNum, sizeof(packetBuffer.AckNum), "%d", packet->AckNum);

	snprintf(buffer, sizeof(PacketBuffer), "%s %s %s %s %s", packetBuffer.Type, packetBuffer.SeqNum,
		packetBuffer.WindowSize, packetBuffer.AckNum, packetBuffer.Data);

	printf("Packetize: Buffer:\n%s\n", buffer);
	printf("Packetize: strlen(buffer) = %d\n\n", (int)strlen(buffer));
}

void Unpacketize(char* buffer, Packet* packet) {
	char *end;

	//copy content from Packet to PacketBuffer and change ints to char[]
	packet->Type = strtol(buffer, &end, 10); 
	packet->SeqNum = strtol(end, &end, 10);
	packet->WindowSize = strtol(end, &end, 10);
	packet->AckNum = strtol(end, &end, 10);
	memcpy(packet->Data, end+1, sizeof(packet->Data));
	
	printf("Unpacketizing:\n");
	printf("packet.Type = %d\n", packet->Type);
	printf("packet.SeqNum = %d\n", packet->SeqNum);
	printf("packet.WindowSize = %d\n", packet->WindowSize);
	printf("packet.AckNum = %d\n", packet->AckNum);
	printf("packet.Data = %s\n\n", packet->Data);
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
        file.close();
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
