#include <string>
#include <thread>
#include "server.h"

#define SUB 0
#define PUB 1
#define SUB_ARGS 3
#define PUB_ARGS 4

#define SUB_SIGNAL "0000000231"
#define PUB_SIGNAL "0000000014"

#define SUCCESS "0000000512"

#define DEFAULT_SIZE 10
#define BUFFER_SIZE 2048

using namespace std;

int main() {
    Server server;
    
    SOCKET newClient;
    server.welcomeClient(&newClient);

    char buf[BUFFER_SIZE] = {'\0'};
    server.recvMsg(newClient, buf, DEFAULT_SIZE);
    string msg = string(buf);
    cout << "msg: " << msg << endl;
    if (msg.compare(SUB_SIGNAL) == 0) {

    }
    else if (msg.compare(PUB_SIGNAL) == 0) {
        // Getting the topic of the message
        server.recvMsg(newClient, buf, DEFAULT_SIZE);
        int msgSize = stoi(string(buf));
        server.recvMsg(newClient, buf, msgSize);
        cout << "Server read: Topic: " << buf << endl;
        
        // Getting the message itself
        server.recvMsg(newClient, buf, DEFAULT_SIZE);
        msgSize = stoi(string(buf));
        server.recvMsg(newClient, buf, msgSize);
        cout << "Server read: Message: " << buf << endl;
        
        // Sending Acknowledgement
        server.sendMsg(newClient, SUCCESS, DEFAULT_SIZE);
    }

    server.close();
}