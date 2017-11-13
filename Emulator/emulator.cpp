#include "../Library/client.h"
#include "../Library/server.h"
#include "../Library/socketwrappers.h"
#include "../Library/library.h"
#include "emulator.h"

#define CLIENTIP 0
#define CLIENTPORT 1
#define EMULATORIP 2
#define EMULATORPORT 3
#define SERVERIP 4
#define SERVERPORT 5

#define PATH "./Client_files/"
#define BUFLEN 1024

using namespace std;

bool getConfig();
string config[BUFLEN];

int main (int argc, char *argv[]) {
    if(!getConfig()){
      perror("Could not get configs");
    }

    string ipp = config[EMULATORIP];
    const char *ip = ipp.c_str();
    string portt = config[EMULATORPORT];
    int port = atoi(portt.c_str());

    Client *outTX = new Client(ip,port);

    string svrport = config[SERVERPORT];
    int srvport = atoi(svrport.c_str());

    Server *inTX = new Server(srvport);

    Cmd cmd;
    int command = 0;
    string ss = config[CLIENTIP];
    char *filename;
    cout << "Enter command: " << endl;
    cin >> filename;
    cmd = CreateCmd(command, filename);
    outTX->SendCmd(cmd);

    Cmd ccmd;

    ccmd = inTX->WaitCommand();



}
bool getConfig(){
     ifstream file;
     file.open("config");
     //check if file exists
        if (!file){
            perror("Files does not exist");
        } else {
            int i;
                for(i=0; i <SERVERPORT; i++){
                    getline(file, config[i]);
                    cout << config[i];
                }
                return true;
        }
  return false;
}
