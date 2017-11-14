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
#define NUM_THREADS 4

#define PATH "./Client_files/"
#define BUFLEN 1024

using namespace std;

void getConfig();
int rando();
bool same(int i,int arr[]);
char *ParseString(string str);
bool discard(int arr[]);

string config[BUFLEN];

int main (int argc, char *argv[]) {
    pthread_t threads[NUM_THREADS];
    int sc,cs,ss,cc;
    int rate = atoi(argv[1]);
    int BER[rate];
    //check arguments
    if (argc != 2){
      perror("Error: ./file [BER]");
    }
    //get ip and port from config file
    getConfig();
    //populate BER array
    int i;
    for(i = 0; i<(sizeof(BER)/sizeof(int)); i++){
        BER[i] = rando();
        cout << BER[i] << endl;
          if (same(BER[i],BER)){
            do {
              BER[i] = rando();
            } while (!same(BER[i],BER));
          }
      }
      // test discard
      /*size_t k;
      for (k = 0; k<(sizeof(BER)/sizeof(int)); k++){
        cout << discard(BER) << endl;
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
    sc = pthread_create(&threads[0], NULL, scStart, NULL);
    cs =
    ss =
    cc =
    Server *serverinclient = new Server(7006);
    Client *clientoutserver = new Client(connectionIP,7008);
    Server *serverinserver = new Server(7007);
    Client *clientoutclient = new Client(connectionIP, 7005);
}
void getConfig(){
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

int rando(){
  return rand() % 100 +1;
}

bool same(int val,int arr[]){
  size_t i;
  for(i = 0; i<(sizeof(arr)/sizeof(int)); i++){
      if (arr[i] == val){
        return true;
      }
  }
  return false;
}

bool discard(int arr[]){
  size_t i;
  int val = rando();
  cout << "checking value with" << val << endl;
  for(i = 0; i<(sizeof(arr)/sizeof(int)); i++){
    if(same(val,arr)){
      i = (sizeof(arr)/sizeof(int));
      return true;
    }
  }
}

