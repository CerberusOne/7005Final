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
#define LOG "logs"

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

	FILE *logfile;
	if((logfile = fopen(LOG, "a")) == NULL) {
		perror("file doesn't exist\n");
        }


	filename = ParseString("config");
	strcpy(configpath, ROOT);
	strcat(configpath, filename);
	GetConfig(configpath, config);

	time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	printf("LOG DATE: %d/%d/%d %d:%d:%d \n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	fprintf(logfile, "LOG DATE: %d/%d/%d %d:%d:%d \n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

	//change IP back to before
	Client *transferConnection = new Client(ParseString(config[EMULATORIP]), atoi(ParseString(config[EMULATORPORT2])));//Client object for transfers
	Server *commandConnection = new Server(atoi(ParseString(config[SERVERPORT])));					//Server object for commands

	SetNonBlocking(commandConnection->GetSocket());
	SetNonBlocking(transferConnection->GetSocket());

	cout <<"Connecting to IP: " << config[EMULATORIP] << endl;
	fprintf(logfile,"Connecting to IP: %s\n", config[EMULATORIP].c_str());
	cout <<"Port: "<< config[EMULATORPORT2] << endl;
	fprintf(logfile,"Connecting to Port: %s\n", config[EMULATORPORT2].c_str());

	//Client *transferConnection = new Client(commandConnection->GetTransferIP(), 7007);//Client object for transfers
//
	//fix seg faults
		//change to non-blocking
		//close sockets if after sending EXIT ACK
		//close sockets regardless

	fclose(logfile);
	do{
		memset(&cmd, 0, sizeof(Cmd));
		if((logfile = fopen(LOG, "a")) == NULL) {
			perror("file doesn't exist\n");
       		}
		printf("Waiting for command\n");
		//cmd = commandConnection->WaitCommand();					//Wait for the client
		rRecvCmd(commandConnection->GetSocket(), &cmd);

		//cmd = RecvCmd(commandConnection->GetSocket());
		printf("Type: %d\n",cmd.type);
		fprintf(logfile,"Type: %d\n",cmd.type);
		printf("Type: %d\n",cmd.type);
		fprintf(logfile,"Type: %d\n",cmd.type);
		printf("Filename: %s\n",cmd.filename);
		fprintf(logfile,"Filename: %s\n",cmd.filename);

		strcpy(path, PATH);
		strcat(path, cmd.filename);

		if(cmd.type == SEND) {
			RecvFile(transferConnection->GetSocket(), path, logfile, cmd.filesize);
		} else if (cmd.type == GET) {
			SendFile(transferConnection->GetSocket(), path, logfile, cmd.filesize);
		}
    fclose(logfile);

	} while (cmd.type != EXIT);

	close(transferConnection->GetSocket());
	close(commandConnection->GetSocket());


	return 0;
}
