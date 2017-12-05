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

#define ROOT "../"

#define BUFLEN 1024

using namespace std;

bool Discard(double probability);

string config[BUFLEN];

int main (int argc, char *argv[]) {
	Cmd cmd;
	Packet packet;
	int bytesRecv = 0;
	int bytesSent = 0;
	double passed;
	double delay = atoi(argv[2]);
	bool timer = true;
	clock_t start;
	char *filename, configpath[BUFLEN];
	time_t t;
	srand((unsigned) time(&t));
	int rate = atoi(argv[1]);
	/**char ratestr;
	*char delaystr;
	cout <<"Please enter the delay" << endl;
	cin >> delaystr;
	cout <<"Please enter the BER" << endl;
	cin >> ratestr;
	rate = atoi(ratestr);
	delay = atoi(delaystr);*/
	if (argc != 3){
            cout << argc << endl;
      		perror("Error: ./file [BER] [DELAY]");
					exit(0);
    	}

	filename = ParseString("config");
	strcpy(configpath, ROOT);
	strcat(configpath, filename);
	GetConfig(configpath, config);

	//for testing purposes
	//string ipemulator = config[SERVERIP];
	//  const char *connectionIP = ipemulator.c_str();

	//setup sockets, ORDER MATTERS
	Server *serverData = new Server(atoi(ParseString(config[EMULATORPORT2])));
	Client *serverCmd = new Client(ParseString(config[SERVERIP]),atoi(ParseString(config[SERVERPORT])));

	cout <<"Connecting to IP: " << config[SERVERIP] << endl;
	cout <<"Port: "<< config[SERVERPORT] << endl;

	Server *clientCmd = new Server(atoi(ParseString(config[EMULATORPORT1])));
	Client *clientData = new Client(ParseString(config[CLIENTIP]), atoi(ParseString(config[CLIENTPORT])));

	cout <<"Connecting to IP: " << config[CLIENTIP] << endl;
	cout <<"Port: "<< config[CLIENTPORT] << endl;

	cout <<"BER set to: " << rate << endl;
	cout <<"Delay set to: " << delay << endl;
	//set all sockets to non-blocking
	SetNonBlocking(clientData->GetSocket());
	SetNonBlocking(clientCmd->GetSocket());
	SetNonBlocking(serverData->GetSocket());
	SetNonBlocking(serverCmd->GetSocket());



	//char* buffer = (char*) malloc(sizeof(char) * sizeof(PacketBuffer) + sizeof(int));
	char* buffer = (char*) malloc(sizeof(char) * sizeof(PacketBuffer));



	//check all ports for data
	while(1){

		//reset cmd and packet for next recv
		//cmd = {0};
		//packet = {0};

		memset (&cmd, 0, sizeof(cmd));
		memset (&packet, 0, sizeof(packet));
		//CMD--------------------------------------------------

		//check if there is a new command from client
		if((bytesRecv = recv(clientCmd->GetSocket(), &cmd, sizeof(Cmd), 0) > 0)) {
			if (!Discard(rate)){
				start = clock();
				printf("Delay started\n");
				printf("Client Command found\n");
				printf("\tCmd Type: %d\tFilename: %s\n", cmd.type, cmd.filename);
				//wait timer ends
				while(timer){
				  passed = (clock() - start) / CLOCKS_PER_SEC;
				    //If timer is not over
				    if (passed >= delay){
				      cout << "Delay End:\n " << passed << endl;
				      timer = false;
				    }
				}
				timer = true;
				//pass the command on to the server
				if((bytesSent = send(serverCmd->GetSocket(), &cmd, sizeof(Cmd), 0)) > 0) {
					printf("Cmd sent to server\n");
				} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
					perror("Send serverCmd failed");
				}
			} else {
				printf("Packet Discarded\n");
			}
		} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
			perror("Recv clientCmd failed");
		}


		//check if there is a command ACK from server
		if((bytesRecv = recv(serverCmd->GetSocket(), &cmd, sizeof(Cmd), 0) > 0)) {
			if (!Discard(rate)){
			printf("Server Command ACK found\n");
			start = clock();
			printf("Delay started\n");

			//wait timer ends
			while(timer){
			  passed = (clock() - start) / CLOCKS_PER_SEC;
			    //If timer is not over
			    if (passed >= delay){
			      cout << "Delay End:\n " << passed << endl;
			      timer = false;
			    }
			}
			timer = true;
			//pass the command ACK on to the client
			if((bytesSent = send(clientCmd->GetSocket(), &cmd, sizeof(Cmd), 0)) > 0) {
				printf("Cmd ACK sent to client\n");
				if(cmd.type == EXIT){
					printf("Exit command received from server\n");
					break;
				}
			} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
				perror("Send clientCmd failed");
			}

				//exit if server sends exit command, check for EXIT ACK from server after testing
				if(cmd.type == EXIT){
					exit(1);
				}
			} else {
				printf("Packet Discarded\n");
			}
		 }else if(errno != EAGAIN || errno != EWOULDBLOCK) {
			perror("Recv serverCmd failed");
		}



		//DATA----------------------------------------------

		//check if there is a data packet from client

		//if((bytesRecv = read(clientData->GetSocket(), &packet, sizeof(Packet))) > 0) {
		//if((bytesRecv = recv(clientData->GetSocket(), &packet, sizeof(Packet), MSG_WAITALL)) > 0) {
		if((bytesRecv = read(clientData->GetSocket(), buffer, sizeof(PacketBuffer))) > 0) {
			printf("\nRecv Client Data:\n");
			printf("BytesRecv from client: %d\n", bytesRecv);

			Packet recvp;
			Unpacketize(buffer, &recvp);


			if (!Discard(rate)){
				printf("Client data found\n");
				//printf("buffer: %s\n", buffer);
				//printf("\tPacket Type: %d\t", packet.Type);
				start = clock();
				printf("Delay started\n");

				//wait timer ends
				while(timer){
				  passed = (clock() - start) / CLOCKS_PER_SEC;
				    //If timer is not over
				    if (passed >= delay){
				      cout << "Delay End\n " << endl;
				      timer = false;
				    }
				}
				timer = true;

				//pass the packet to the server
				//if((bytesSent = send(serverData->GetSocket(), &packet, sizeof(Packet), 0)) > 0) {
				if((bytesSent = write(serverData->GetSocket(), buffer, sizeof(PacketBuffer))) > 0 ){
				//if((bytesSent = send(serverData->GetSocket(), buffer, sizeof(buffer), 0)) > 0) {
					printf("Buffer: %s\n", buffer);
					printf("Packet sent to server\n");
				} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
					perror("Send serverData failed\n");
				}
			} else {
				printf("Packet Discarded\n");
			}

			memset(buffer, 0, (int)sizeof(buffer));
		} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
			perror("Recv clientData failed");
		}
		




		//check if there is a data ACK from server
		//if((bytesRecv = recv(serverData->GetSocket(), &packet, sizeof(Packet), MSG_WAITALL) > 0)) {
		//if((bytesRecv = recv(serverData->GetSocket(), buffer, sizeof(buffer), 0) > 0)) {
		if((bytesRecv = read(serverData->GetSocket(), buffer, sizeof(PacketBuffer)) > 0)) {
			if (!Discard(rate)){
			printf("Server data ACK found\n");

			start = clock();
			printf("Delay started\n");

			//wait timer ends
			while(timer){
			  passed = (clock() - start) / CLOCKS_PER_SEC;
			    //If timer is not over
			    if (passed >= delay){
			      cout << "Delay End:\n " << passed << endl;
			      timer = false;
			    }
			}
			timer = true;

			//pass the command ACK on to the client
			//if((bytesSent = send(clientData->GetSocket(), &packet, sizeof(Packet), 0)) > 0) {
			//if((bytesSent = send(clientData->GetSocket(), buffer, sizeof(buffer), 0)) > 0) {
			if((bytesSent = write(clientData->GetSocket(), buffer, sizeof(PacketBuffer))) > 0) {
				printf("Data ACK sent to client\n");
			} else if(errno != EAGAIN || errno != EWOULDBLOCK) {
				perror("Send clientData failed");
			}
			if(cmd.type == EOT){
				printf("EOT received from Server");
			}
			} else{
				printf("Packet Discarded: %d\n", packet.AckNum);
			}
			
			memset(buffer, 0, (int) sizeof(buffer));
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

bool Discard(double probability){
	probability /= 100;
	return rand() < probability * ((double)RAND_MAX + 1.0);
}
