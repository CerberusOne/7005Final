#include "client.h"

using namespace std;

Client::Client(const char* ip, int port) {
    servfd = Socket(AF_INET, SOCK_STREAM, 0);
    ConfigClientSocket(&servaddr, ip, port);
    Connect(servfd, servaddr);
}


Client::Client(struct sockaddr_in *bindaddr, struct sockaddr_in *transferaddr) {
    servfd = Socket (AF_INET, SOCK_STREAM, 0);
    Bind(servfd, bindaddr);
    Connect(servfd, *transferaddr);
}

int Client::GetSocket() {
    return servfd;
}
