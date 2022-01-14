#include <string>
#include <thread>
#include <winsock2.h>
#include <windows.h>
#include "client.h"

#define SUB 0
#define PUB 1
#define SUB_ARGS 3
#define PUB_ARGS 4

#define SUB_SIGNAL "0000000231"
#define PUB_SIGNAL "0000000014"

#define SUCCESS "0000000512"

#define DEFAULT_SIZE 10

using namespace std;
bool status;

void throwUsageError() {
    cerr << endl << "Usage:\tpublish [Topic] [\"Message\"]" << endl
    << "      \tor"
    << endl << "      \tsubscribe [Topic 1] [Topic 2] ... [Topic N]" << endl;
    exit(1);
}

int sendRequest(int action, int argc, char *argv[]) {
    Client client;
    cout << "Here" << endl;
    string msg = (action == SUB ? SUB_SIGNAL : PUB_SIGNAL);
    client.sendMsg(&msg[0], DEFAULT_SIZE);

    if (action == PUB) {
        // Sending the topic of the message
        msg = string(argv[2]);
        string msgSize = to_string(msg.size());
        string spaces = "          ";
        msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
        client.sendMsg(&msgSize[0], DEFAULT_SIZE);
        client.sendMsg(&msg[0], msg.size());

        // Sending the message itself
        msg = string(argv[3]);
        for (int i = 4; i < argc; ++i) {
            msg = msg.append(string(argv[i]));
        }
        
        msgSize = to_string(msg.size());
        spaces = "          ";
        msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
        client.sendMsg(&msgSize[0], DEFAULT_SIZE);
        client.sendMsg(&msg[0], msg.size());
    } else {

    }

    char buf[2048];
    client.recvMsg(buf, DEFAULT_SIZE);
    msg = string(buf);
    if (msg.compare(SUCCESS) == 0) {
        client.close();
        return 0;
    }
    return 1;
}

int main(int argc, char *argv[]) {

    int action;

    if (argc < 2)
        throwUsageError();

    string inputCommand = string(argv[1]);

    if (inputCommand.compare("subscribe") == 0)
        action = SUB;
    else if (inputCommand.compare("publish") == 0)
        action = PUB;
    else {
        cerr << "'" << inputCommand << "' not recognised" << endl;
        throwUsageError();
    }

    if (action == PUB) {
        status = false;
        if (argc < PUB_ARGS)
            throwUsageError();
        if (sendRequest(action, argc, argv) == 0) {
            status = true;
            cout << "Your message got published!" << endl;
        }
    }

    return 0;
}