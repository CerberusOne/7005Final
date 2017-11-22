#include "../Library/client.h"
#include "../Library/server.h"
#include "../Library/socketwrappers.h"
#include "../Library/library.h"
#include "emulator.h"

#define CLIENTIP 0
#define CLIENTPORT 1
#define EMULATORIP 2
#define EMULATORPORT1 3
#define EMULATORPORT2 4
#define SERVERIP 5
#define SERVERPORT 6
#define DATA  0
#define ACK		1
#define EOT		2

#define BUFLEN 1024

using namespace std;

void GetConfig();
int Rando();
//bool Same(int i,int *arr,int size);
//char *ParseString(string str);
//bool Discard(int *arr,int size);

string config[BUFLEN];

int main (int argc, char *argv[]) {
	GetConfig();
    	
	//for testing purposes
    	string ipemulator = config[SERVERIP];
  	const char *connectionIP = ipemulator.c_str();

	//setup sockets, ORDER MATTERS
    	Server *serverData = new Server(7007);
    	Client *serverCmd = new Client(connectionIP,7008);
    	
	Server *clientCmd = new Server(7006);
	Client *clientData = new Client(connectionIP, 7005);

	//set all sockets to non-blocking
      	SetNonBlocking(clientData->GetSocket());
	SetNonBlocking(clientCmd->GetSocket());
	SetNonBlocking(serverData->GetSocket());
      	SetNonBlocking(serverCmd->GetSocket());

	//setup command and packet objects to read/write from sockets with
	Cmd cmd;
	Packet packet;
	int bytesRecv = 0;
	int bytesSent = 0;

	//check all ports for data
    	while(1){
		//reset cmd and packet for next recv
		cmd = {0};
		packet = {0};

		//CMD--------------------------------------------------

		//check if there is a new command from client
		if((bytesRecv = recv(clientCmd->GetSocket(), &cmd, sizeof(Cmd), 0) > 0)) {
			printf("Client Command found\n");
			printf("\tCmd Type: %d\tFilename: %s\n", cmd.type, cmd.filename);

			//pass the command on to the server
			if((bytesSent = send(serverCmd->GetSocket(), &cmd, sizeof(Cmd), 0)) > 0) {
				printf("Cmd sent to server\n");
			} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
				perror("Send serverCmd failed");
			}
				
			//exit if client sends exit command, check for EXIT ACK from server after testing
			if(cmd.type == EXIT)
				break;
		} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
			perror("Recv clientCmd failed");
		}
		
		
		//check if there is a command ACK from server
		if((bytesRecv = recv(serverCmd->GetSocket(), &cmd, sizeof(Cmd), 0) > 0)) {	
			printf("Server Command ACK found\n");

			//pass the command ACK on to the client
			if((bytesSent = send(clientCmd->GetSocket(), &cmd, sizeof(Cmd), 0)) > 0) {
				printf("Cmd ACK sent to client\n");
			} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
				perror("Send clientCmd failed");
			}
		} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
			perror("Recv serverCmd failed");
		}



		//DATA----------------------------------------------
		
		//check if there is a data packet from client
		if((bytesRecv = recv(clientData->GetSocket(), &packet, sizeof(Packet), 0) > 0)) {
			printf("Client data found\n");
			printf("\tPacket Type: %d\t", packet.Type);

			//pass the command on to the server
			if((bytesSent = send(serverData->GetSocket(), &packet, sizeof(Packet), 0)) > 0) {
				printf("Packet sent to server\n");
			} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
				perror("Send serverData failed");
			}
		} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
			perror("Recv clientData failed");
		}

		
		//check if there is a data ACK from server
		if((bytesRecv = recv(serverData->GetSocket(), &packet, sizeof(Packet), 0) > 0)) {	
			printf("Server data ACK found\n");

			//pass the command ACK on to the client
			if((bytesSent = send(clientData->GetSocket(), &packet, sizeof(Packet), 0)) > 0) {
				printf("Data ACK sent to client\n");
			} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
				perror("Send clientData failed");
			}
		} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
			perror("Recv serverData failed");
		}
	}



	close(clientCmd->GetSocket());
	close(serverCmd->GetSocket());
	close(clientData->GetSocket());
	close(serverData->GetSocket());

	return 0;
}

void GetConfig(){
	ifstream file;
     	file.open("config");
     	//check if file exists
        if (!file){
        	perror("Files does not exist");
       	} else {
        	int i;
                for(i=0; i <SERVERPORT; i++){
                	getline(file, config[i]);
                    	//cout << config[i] << endl;
                }
        }
}


