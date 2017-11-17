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
void Timeout();

int main (int argc, char *argv[]) {
	Cmd cmd;			//command structure
	char path[BUFLEN];	//path of files according to the root directory of the executable

	Server *commandConnection = new Server(7005);					//Server object for commands
	SetBlocking(commandConnection->GetSocket());
	Client *transferConnection = new Client(commandConnection->GetTransferIP(), 70005);//Client object for transfers
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

	while(1){
		//backup FD_SET
		memcpy(&master, &backup, sizeof(backup));

		cout << "waiting for event" << endl;

		rc = select (fdmax + 1, &master, NULL, NULL, timeout);
		if (rc < 0){
		perror("ERROR: Select()");
		exit(4);
		}

		int i;
		for (i = 0; i <= fdmax; i++){
				//if descriptor is in the set
				if (FD_ISSET(i, &master)){

						if(i == commandConnection->GetSocket(){

						} else if (i == transferConnection->GetSocket()){

						}

				}
		}
		if (rc == 0){
			Timeout();
		}
	}

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

void Timeout(){

}
