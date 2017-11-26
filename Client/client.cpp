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

#define CLIENTIP 0
#define CLIENTPORT 1
#define EMULATORIP 2
#define EMULATORPORT1 3
#define EMULATORPORT2 4
#define SERVERIP 5
#define SERVERPORT 6

#define PATH "./Client_files/"
#define ROOT "../"

using namespace std;

bool isValidFile(char *cfilename);
bool isCommand(string strcommand, int &command);

string config[BUFLEN];

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

	char *configFilename, configpath[BUFLEN];

	configFilename = ParseString("config");
	strcpy(configpath, ROOT);
	strcat(configpath, configFilename);
	GetConfig(configpath, config);


	Client *commandConnection = new Client(ParseString(config[EMULATORIP]), atoi(ParseString(config[EMULATORPORT1])));
	Server *transferConnection = new Server(atoi(ParseString(config[CLIENTPORT])));

	cout <<"Connecting to IP: " << config[EMULATORIP] << endl;
	cout <<"Port: "<< config[EMULATORPORT1] << endl;

	//SetNonBlocking(commandConnection->GetSocket());
	SetNonBlocking(transferConnection->GetSocket());


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
