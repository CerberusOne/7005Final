/*------------------------------------------------------------------------------------------------
-- SOURCE FILE: client.cpp
--
-- PROGRAM:     COMP7005 - Assignment 1 FTP
--
-- FUNCTIONS:   void RecvFile(int socket, char* filename);
--				void SendFile(int socket, char *filename);
--				bool isValidFile(char *cfilename);
--				bool isCommand(string strcommand, int &command);
--				char *ParseString(string str);
--
-- DATE:        Oct 2, 2017
--
-- DESIGNER:    Aing Ragunathan
--
-- PROGRAMMER:  Aing Ragunathan
--
-- NOTES:       Server class methods for creating and using a client for the FTP program
--------------------------------------------------------------------------------------------------*/

#include "../A1/client.h"
#include "../A1/server.h"
#include "../A1/socketwrappers.h"

#define PATH "./Client_files/"

using namespace std;

void RecvFile(int socket, char* filename);
void SendFile(int socket, char *filename);
bool isValidFile(char *cfilename);
bool isCommand(string strcommand, int &command);
char *ParseString(string str);
int findLen(char* str); 


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
-- RETURNS:    int
--
-- NOTES:      Constructor for a Server
----------------------------------------------------------------------------------------------- */
int main (int argc, char *argv[]) {
	int command;
	Cmd cmd;
	string filename, strcommand, serverIP, serverPort;
	char *cfilename, path[BUFLEN];

	//get ip, port and transfer port 
	cout << "Enter server IP:" << endl;
	cin >> serverIP; 

	Client *commandConnection = new Client(ParseString(serverIP), 7005);
	Server *transferConnection = new Server(70005);

	do{
		fflush(stdin);

		//get user input and validate
		do {
			cout << "Enter command: " << endl;
			cin >> strcommand; 
		} while(!isCommand(strcommand, command));

		//get user input for filename and validate
		if(command != EXIT){
			//get filename
			do {
				cout << "Enter fileame: " << endl;
				cin >> filename; 
				cfilename = ParseString(filename);
				strcpy(path, PATH);
				strcat(path, cfilename);
			} while (command == SEND && !isValidFile(path));
		} else {
			cfilename = ParseString("exit");
		}

		//create a command
		cmd = CreateCmd(command, cfilename);
		//send the command
		commandConnection->SendCmd(cmd);
		//send or receive the file
		if(cmd.type == SEND) {
			printf("sending file\n");
			SendFile(transferConnection->GetSocket(), path);
		} else if(cmd.type == GET) {
			printf("getting file\n");
			RecvFile(transferConnection->GetSocket(), path);
		} else {
			printf("exiting\n");
		}
	} while (cmd.type != EXIT);

	return 0;
}


/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   ParseString
--
-- DATE:       Oct 2, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  void ParseString(string str)
--
-- PARAMETER:  string str 	- string to be parsed
--
-- RETURNS:    a char array version of the string
--
-- NOTES:      Converts a string to a char array
----------------------------------------------------------------------------------------------- */
char *ParseString(string str) {
	char *cstr;

	cstr = new char[str.length() + 1];
	strcpy(cstr, str.c_str());

	return cstr;
}

/*-----------------------------------------------------------------------------------------------
-- FUNCTION:   isCommand
--
-- DATE:       Oct 2, 2017
--
-- DESIGNER:   Aing Ragunathan
--
-- PROGRAMMER: Aing Ragunathan
--
-- INTERFACE:  bool isCommand(string strcommand, int &command)
--
-- PARAMETER:  	string strcommand 	- command in string form
--				int command 		- command in int form
--
-- RETURNS:    true if the command is valid
--
-- NOTES:      Converts a string command to an int command and validates it
----------------------------------------------------------------------------------------------- */
bool isCommand(string strcommand, int &command) {
	if(strcommand == "send") {
		command = SEND;
		return true;
	} else if(strcommand == "get") {
		command = GET;
		return true;
	} if(strcommand == "exit") {
		command = EXIT;
		exit(1);
		return true;
	} else {
		cout << strcommand << endl;
		return false;
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
