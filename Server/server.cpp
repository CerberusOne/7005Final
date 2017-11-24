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

#define CLIENTIP 0
#define CLIENTPORT 1
#define EMULATORIP 2
#define EMULATORPORT1 3
#define EMULATORPORT2 4
#define SERVERIP 5
#define SERVERPORT 6

#define PATH "./Server_files/"
#define ROOT "../"

//void SendFile(int socket, char *filename);
//void RecvFile(int socket, char* filename);


using namespace std;

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
-- RETURNS:    void
--
-- NOTES:      Constructor for a Server
----------------------------------------------------------------------------------------------- */
int main (int argc, char *argv[]) {
	Cmd cmd;			//command structure
	char path[BUFLEN],*filename, configpath[BUFLEN];	//path of files according to the root directory of the executable

	filename = ParseString("config");
	strcpy(configpath, ROOT);
	strcat(configpath, filename);
	GetConfig(configpath, config);

	//change IP back to before
	Client *transferConnection = new Client(ParseString(config[EMULATORIP]), atoi(ParseString(config[EMULATORPORT2])));//Client object for transfers
	Server *commandConnection = new Server(atoi(ParseString(config[SERVERPORT])));					//Server object for commands

	cout <<"Connecting to IP: " << config[EMULATORIP] << endl;
	cout <<"Port: "<< config[EMULATORPORT2] << endl;

	//Client *transferConnection = new Client(commandConnection->GetTransferIP(), 7007);//Client object for transfers

	//fix seg faults
		//change to non-blocking
		//close sockets if after sending EXIT ACK
		//close sockets regardless
	do{
		//cmd = commandConnection->WaitCommand();					//Wait for the client
		cmd = RecvCmd(commandConnection->GetSocket());
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

	close(transferConnection->GetSocket());
	close(commandConnection->GetSocket());

	return 0;
}
