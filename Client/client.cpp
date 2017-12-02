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
#define LOGPATH "logs"

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

	FILE *logs;
	if((logs = fopen(LOGPATH, "a")) == NULL) {
		perror("file doesn't exist\n");
	}

	configFilename = ParseString("config");
	strcpy(configpath, ROOT);
	strcat(configpath, configFilename);
	GetConfig(configpath, config);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("LOG DATE: %d/%d/%d %d:%d:%d \n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(logs, "LOG DATE: %d/%d/%d %d:%d:%d \n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	Client *commandConnection = new Client(ParseString(config[EMULATORIP]), atoi(ParseString(config[EMULATORPORT1])));
	Server *transferConnection = new Server(atoi(ParseString(config[CLIENTPORT])));

	cout <<"Connecting to IP: " << config[EMULATORIP] << endl;
	fprintf(logs,"Connecting to IP: %s\n", config[EMULATORIP].c_str());
	cout <<"Port: "<< config[EMULATORPORT1] << endl;
	fprintf(logs,"Connecting to Port: %s\n", config[EMULATORPORT1].c_str());

	SetNonBlocking(commandConnection->GetSocket());
	SetNonBlocking(transferConnection->GetSocket());
	fclose(logs);

	do{
		fflush(stdin);
    		FILE *logs;
		if((logs = fopen(LOGPATH, "a")) == NULL) {
			perror("file doesn't exist\n");
	    	}

		//get user input and validate
		do {
			cout << "Enter command: " << endl;
			cin >> strcommand;
			fprintf(logs,"Enter command: %s\n", strcommand.c_str());
		} while(!isCommand(strcommand, command));

		//get user input for filename and validate
		if(command != EXIT){
			//get filename
			do {
				cout << "Enter fileame: " << endl;
				cin >> filename;
				fprintf(logs,"Enter filename %s\n", filename.c_str());
				cfilename = ParseString(filename);
				strcpy(path, PATH);
				strcat(path, cfilename);
			} while (command == SEND && !isValidFile(path));
		} else {
			cfilename = ParseString("exit");
		}

		//create a command
		cmd = CreateCmd(command, cfilename);

		rSendCmd(commandConnection->GetSocket(), &cmd);
		
		//UNRELIABLIY send the command
		//SendCmd(commandConnection->GetSocket(), cmd);

		//send or receive the file
		if(cmd.type == SEND) {
			printf("sending file\n");
			fprintf(logs,"sending file\n");
			SendFile(transferConnection->GetSocket(), path, logs);
		} else if(cmd.type == GET) {
			printf("getting file\n");
			fprintf(logs,"Getting file\n");
			RecvFile(transferConnection->GetSocket(), path, logs);
		} else {
			printf("exiting\n");
			fprintf(logs,"exiting\n");
		}
		fclose(logs);
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
		return true;
	} else {
		cout << strcommand << endl;
		return false;
	}
}
