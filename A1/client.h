#ifndef CLIENT_H
#define CLIENT_H

#include "socketwrappers.h"
#include <netinet/in.h>
#include <iostream> 
#include <strings.h>
#include <stdlib.h>
#include <cstring>

using namespace std;

class Client
{
public:
	Client(const char* ip, int port);
	Client(struct sockaddr_in *bindaddr, struct sockaddr_in *transferaddr);
	~Client() {
		cout << "Closing client" << endl <<  endl;
	}
    bool SendCmd(Cmd cmd);
    //int GetTransferSocket();
    int GetSocket();
private: 
    int servfd;
    struct sockaddr_in servaddr;
};

#endif


