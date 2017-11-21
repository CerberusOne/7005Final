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
int main (int argc, char *argv[]) {
	Cmd cmd;			//command structure 
	char path[BUFLEN];	//path of files according to the root directory of the executable

	Server *commandConnection = new Server(7008);					//Server object for commands	
	Client *transferConnection = new Client(commandConnection->GetTransferIP(), 7007);//Client object for transfers
	
	SetNonBlocking(commandConnection->GetSocket());
	SetNonBlocking(transferConnection->GetSocket());
	
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





