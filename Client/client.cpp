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

#include "../Library/socketwrappers.h"
#include "../Library/library.h"
#include "../Library/client.h"
#include "../Library/server.h"


#define PATH "./Client_files/"

using namespace std;

//void RecvFile(int socket, char* filename);
//void SendFile(int socket, char *filename);
bool isValidFile(char *cfilename);
bool isCommand(string strcommand, int &command);
void SetBlocking(int socket);
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
	SetBlocking(commandConnection->GetSocket());
	Server *transferConnection = new Server(70005);
	SetBlocking(transferConnection->GetSocket());

	fd_set backup,master;
	struct timeval timeout;
	int fdmax;
	//empty set
	FD_ZERO(&backup);
	FD_ZERO(&master);

	//Add sockets to set
	FD_SET(commandConnection->GetSocket(),&backup);
	FD_SET(transferConnection->GetSocket(),&backup);

	//track the biggest file descriptor
	fdmax = transferConnection->GetSocket();
	//set timeout value to one minute
	timeout.tv_sec = 1 * 60;
	timeout.tv_usec = 0;

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
		SendCmd(commandConnection->GetSocket(), cmd);
		//send or receive the file
		while(1){
		memcpy(&master, &backup, sizeof(backup));

			cout << "waiting for event" << endl;

			rc = select (fdmax + 1, &master, NULL, NULL, timeout);
			if (rc < 0){
			perror("ERROR: Select()");
			exit(4);
			}
			//timeout
			if (rc == 0){
				timeout();
			}

			if (){

			}
			int i;
			for (i = 0; i <= fdmax; i++){
				if (FD_ISSET(i, &master)){
					if (i == transferConnection->GetSocket()){

					}
				}
		}
		}
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

void SetBlocking (int socket){
	rc = fcntl(socket,F_SETFL,O_NONBLOCK);
	if (rc < 0){
		 perror("Error setting socket to non-blocking");
		exit(3);
	}
}
