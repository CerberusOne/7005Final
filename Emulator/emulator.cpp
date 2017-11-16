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

#define BUFLEN 1024

using namespace std;

void GetConfig();
int Rando();
//bool Same(int i,int *arr,int size);
//char *ParseString(string str);
//bool Discard(int *arr,int size);

string config[BUFLEN];

int main (int argc, char *argv[]) {
	    int rc;
      Cmd cmdRX, cmdTX;
      char RXpath[BUFLEN], TXpath[BUFLEN];
    	//int rate = atoi(argv[1]);
    	//int BER[rate];
    	//check arguments
    	if (argc != 2){
      		perror("Error: ./file [BER]");
    	}
    	//get ip and port from config file
    	GetConfig();
    	//populate BER array
    	/*int i;
    	for(i = 0; i<rate; i++){
        	BER[i] = Rando();
        	cout << BER[i] << endl;
          	if (Same(BER[i],BER,rate)){
            		do {
              			BER[i] = Rando();
            		} while (!Same(BER[i],BER,rate));
          	}
      	}*/
      	// test discard
      	/*size_t k;
      	for (k = 0; k<(sizeof(BER)/sizeof(int)); k++){
        	cout << Discard(BER,rate) << endl;
      	}*/

    	//for testing purposes
    	string ipemulator = config[SERVERIP];
    	const char *connectionIP = ipemulator.c_str();

    	//set sockets to non-blocking
    	Server *TXcmd = new Server(7006);
    	rc = fcntl(TXcmd->GetSocket(),F_SETFL,O_NONBLOCK);
    	if (rc < 0){
	    	 perror("Error setting socket to non-blocking");
	    	exit(3);
    	}
    	Client *RXdata = new Client(connectionIP,7008);
      rc = fcntl(RXdata->GetSocket(),F_SETFL,O_NONBLOCK);
    	if (rc < 0){
	    	 perror("Error setting socket to non-blocking");
	    	exit(3);
    	}
    	Server *RXcmd = new Server(7007);
    	rc = fcntl(RXcmd->GetSocket(),F_SETFL,O_NONBLOCK);
    	if (rc < 0){
		      perror("Error setting socket to non-blocking");
	    	    exit(3);
    	}
	    Client *TXdata = new Client(connectionIP, 7005);
      rc = fcntl(TXdata->GetSocket(),F_SETFL,O_NONBLOCK);
    	if (rc < 0){
	    	 perror("Error setting socket to non-blocking");
	    	exit(3);
    	}

    	fd_set listeningset,master;
    	//struct timeval timeout;
    	int fdmax;
    	//empty set
    	FD_ZERO(&listeningset);
    	FD_ZERO(&master);
    	//Add sockets to set
    	FD_SET(TXcmd->GetSocket(),&listeningset);
    	FD_SET(RXcmd->GetSocket(),&listeningset);
      FD_SET(TXdata->GetSocket(),&listeningset);
    	FD_SET(RXdata->GetSocket(),&listeningset);

    	//track the biggest file descriptor
    	fdmax = RXdata->GetSocket();
    	//set timeout value to one minute
    	//timeout.tv_sec = 1 * 60;
    	//timeout.tv_usec = 0;

    	while(1){
		//clock_t start = clock();
		//cout << "Delay started" << endl;
		//copy listeningset to master
		memcpy(&master, &listeningset, sizeof(listeningset));

	    	cout << "waiting for event" << endl;

		//wait
	    	rc = select (fdmax + 1, &master, NULL, NULL, NULL);
	    	if (rc < 0){
		    perror("ERROR: Select()");
		    exit(4);
	    	}
		//This can be used to see if it timed out can be used in the receiver
		//if (rc == 0){	}


	    	int i;
	    	for (i = 0; i <= fdmax; i++){
			//if descriptor is in the set
			if (FD_ISSET(i, &master)){
				/*double passed;
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
				}*/
				//If descriptor is coming from client
				//ADD ERROR HANDLING FOR RECEIVE AND SEND
				if (i == TXcmd->GetSocket()){
					//If BER == 1 discard packet
					//if (!Discard(BER,rate)){
						//Data from client will always be data or EOT
            cout <<"Transmitter command socket" << endl;
            cmdTX = RecvCmd(TXcmd->GetSocket());
            printf("Type: %d\n",cmdTX.type);
            printf("Filename: %d\n",cmdTX.filename);
            SendCmd(RXdata->GetSocket(),cmdTX);
					//}

				} else if(i == RXcmd->GetSocket()){
					//If BER == 1 discard packet
					//if (!Discard(BER,rate)){
					//Data from server will always be an Ack or EOT
          cout <<"Receiver command socket" << endl;
          cmdRX = RecvCmd(RXcmd->GetSocket());
          printf("Type: %d\n",cmdRX.type);
          printf("Filename: %d\n",cmdRX.filename);
          SendCmd(TXdata->GetSocket(),cmdRX);
					//}

				} else if(i == RXdata->GetSocket()){
          cout <<"Receiver data socket" << endl;
          SendFile(TXdata->GetSocket(), cmdTX.filename);

			} else if(i == TXdata->GetSocket()){
        cout <<"Transmitter data socket" << endl;
          SendFile(RXdata->GetSocket(), cmdRX.filename);

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

/*int Rando(){
  	return rand() % 100 +1;
}

bool Same(int val,int *arr,int size){
  	int i;
  	for(i = 0; i<size; i++){
      		if (arr[i] == val){
        		return true;
      		}
  	}
  return false;
}

bool Discard(int *arr,int size){
  	int i;
  	int val = Rando();
  	cout << "checking value with" << val << endl;
  	for(i = 0; i<size; i++){
    		if(Same(val,arr,size)){
      			i = size;
      			return true;
    		}
  	}
}*/
