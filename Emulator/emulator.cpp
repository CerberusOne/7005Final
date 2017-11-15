#include ":w../Library/client.h"
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

#define PATH "./Client_files/"
#define BUFLEN 1024

using namespace std;

void GetConfig();
int Rando();
bool Same(int i,int arr[],int size);
char *ParseString(string str);
bool Discard(int arr[],int size);

string config[BUFLEN];

int main (int argc, char *argv[]) {
	int rc;
    	int rate = atoi(argv[1]);
    	int BER[rate];
    	//check arguments
    	if (argc != 2){
      		perror("Error: ./file [BER]");
    	}
    	//get ip and port from config file
    	GetConfig();
    	//populate BER array
    	int i;
    	for(i = 0; i<rate; i++){
        	BER[i] = r\Rando();
        	cout << BER[i] << endl;
          	if ((Same(BER[i],BER))){
            		do {
              			BER[i] = Rando();
            		} while (!Same(BER[i],BER));
          	}
      	}
      	// test discard
      	/*size_t k;
      	for (k = 0; k<(sizeof(BER)/sizeof(int)); k++){
        	cout << Discard(BER,rate) << endl;
      	}*/

    	//for testing purposes
    	string ipemulator = config[SERVERIP];
    	const char *connectionIP = ipemulator.c_str();

    	/*
    	string porte = config[EMULATORPORT];
    	int eport = atoi(porte.c_str());
    	Server *serverinclient = new Server(eport);

    	string ips = config[SERVERIP];
    	const char *sip = ips.c_str();
    	string Ports = config[SERVERPORT];
    	int sPort = atoi(ports.c_str());
    	Client *clientoutserver = new Client(ips,sPort);

    	string porte2 = config[EMULATORPORT2];
    	int ePort2 = atoi(porte2.c_str());
    	Server *serverinserver = new Server(ePort2);

    	string ipC = config[CLIENTIP];
    	const char *cIp = ipemulator.c_str();
    	string portC = config[CLIENTPORT];
    	int cPort = atoi(portC.c_str());
    	Client *clientoutclient = new Client(ipC, cPort);*/

    	//set sockets to non-blocking
    	Server *INclient = new Server(7006);
    	rc = fcntl(INclient->GetSocket(),F_SETFL,O_NONBLOCK);
    	if (rc < 0){
	    	perrorperror("Error setting socket to non-blocking");
	    	exit(3);
    	}
    	Client *OUTserver = new Client(connectionIP,7008);
    	Server *INserver = new Server(7007);
    	rc = fcntl(INserver->GetSocket(),F_SETFL,O_NONBLOCK);
    	if (rc < 0){
		perror("Error setting socket to non-blocking");
	    	exit(3);
    	}
	Client *OUTclient = new Client(connectionIP, 7005);

    	fd_set listeningset,master;
    	//struct timeval timeout;
    	int fdmax;
    	//empty set
    	FD_ZERO(&listeningset);
    	FD_ZERO(&master);
    	//Add sockets to set
    	FD_SET(INclient->GetSocket(),&listeningset);
    	FD_SET(INserver->GetSocket(),&listeningset);

    	//track the biggest file descriptor
    	fdmax = INserver->GetSocket();
    	//set timeout value to one minute
    	//timeout.tv_sec = 1 * 60;
    	//timeout.tv_usec = 0;

    	while(1){
		clock_t start = clock();
		//copy listeningset to master
		memcpy(&master, &listeningset, sizeof(listeningset));

	    	cout << "waiting for event" << endl;

		//wait
	    	rc = select (fdmax + 1, &master, NULL, NULL, NULL);
	    	if (rc < 0){
		    perror ("ERROR: Select()");
		    exit(4);
	    	}
		//This can be used to see if it timed out can be used in the receiver
		//if (rc == 0){	}

	    	int i;
	    	for (i = 0; i <= fdmax; i++){
			//if descriptor is in the set
			if (FD_ISSET(i, &master)){
				double passed;
				double delay = 60;
				bool timer = true;
				// start timer
				while(timer){
					//(current time) - (start of timer) / CLOCKS_PER_SEC (The Clock() returned unit)
					passed = (clock() - start) / CLOCKS_PER_SEC;
						//If timer is not over
						if (passed >= delay){
							cout << "Seconds have passed: " << passed << endl;
							timer = false;
						}
				}
				//If descriptor is coming from client
				//ADD ERROR HANDLING FOR RECEIVE AND SEND
				if (i == INclient->GetSocket()){
					//If BER == 1 discard packet
					if (!Discard(BER,rate)){
						//Data from client will always be data or EOT
						//Receive packet from client
						RecvFile(INclient->GetSocket(),path);
						//Send packet to server
						SendFile(OUTclient->GetSocket(),path);
					}

				} else if(i == INserver->GetSocket()){
					//If BER == 1 discard packet
					if (!Discard(BER,rate)){
					//Data from server will always be an Ack or EOT
					//Receive packet from server
					RecvFile(INserver->GetSocket(),path);
					//Send Packet to client
					SendFile(OUTserver->GetSocket(),path);
					}

				}
			}
	    }
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

char *ParseString(string str){
    	char *cstr;

    	cstr = new char[str.length() + 1];
    	strcpy(cstr, str.c_str());

    	return cstr;
}

int Rando(){
  	return rand() % 100 +1;
}

bool Same(int val,int arr[],int size){
  	size_t i;
  	for(i = 0; i<size; i++){
      		if (arr[i] == val){
        		return true;
      		}	
  	}
  return false;
}

bool Discard(int arr[],int size){
  	size_t i;
  	int val = Rando();
  	cout << "checking value with" << val << endl;
  	for(i = 0; i<size; i++){
    		if(Same(val,arr)){
      			i = size;
      			return true;
    		}
  	}
}
