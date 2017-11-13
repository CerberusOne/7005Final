#include "../A1/client.h"
#include "../A1/server.h"
#include "../A1/socketwrappers.h"
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
  if(getConfig){

  }
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
