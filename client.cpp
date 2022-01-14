#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <winsock2.h>
#include <windows.h>
#include "client.h"
#include "constants.h"

using namespace std;

bool status;
mutex outputLock;

void throwUsageError() {
    cerr << endl << "Usage:\tpublish [Topic] [\"Message\"]" << endl
    << "      \tor"
    << endl << "      \tsubscribe [Topic 1] [Topic 2] ... [Topic N]" << endl;
    exit(1);
}

void subscribe(string host, string port, int topicIndex, char *argv[]) {
    // Creating a client and sending a subscribe signal:
    Client client(&host[0], &port[0]);
    string msg = SUB_SIGNAL;
    client.sendMsg(&msg[0], DEFAULT_SIZE);

    // Sending the topic of the message:
    msg = string(argv[topicIndex]);
    string msgSize = to_string(msg.size());
    string spaces = "          ";
    msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
    client.sendMsg(&msgSize[0], DEFAULT_SIZE);
    client.sendMsg(&msg[0], msg.size());

    // Receiving the number of messages from the server:
    char buf[BUFFER_SIZE];
    client.recvMsg(buf, DEFAULT_SIZE);

    int messagesNumber = stoi(string(buf));
    
    outputLock.lock();

    cout << endl;
    if (messagesNumber == 0)
        cout << "\tYou have no messages on topic '" << argv[topicIndex] << "':" << endl;
    else {
        cout << "\tMessages on topic '" << argv[topicIndex] << "':" << endl;
        for (int i = 0; i < messagesNumber; ++i) {
            client.recvMsg(buf, DEFAULT_SIZE);
            int messageSize = stoi(string(buf));
            client.recvMsg(buf, messageSize);
            cout << "\t\t" << i << ". " << buf << endl;
        }
    }

    outputLock.unlock();

    client.close();
}

int sendRequest(string host, string port, int action, int argc, char *argv[]) {
    
    if (action == PUB) {
        // Creating a client and sending publish request:
        Client client(&host[0], &port[0]);
        string msg = PUB_SIGNAL;
        client.sendMsg(&msg[0], DEFAULT_SIZE);

        // Sending the topic of the message:
        msg = string(argv[SP_INDEX + 1]);
        string msgSize = to_string(msg.size());
        string spaces = "          ";
        msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
        client.sendMsg(&msgSize[0], DEFAULT_SIZE);
        client.sendMsg(&msg[0], msg.size());

        // Sending the message itself:
        msg = string(argv[SP_INDEX + 2]);
        for (int i = SP_INDEX + 3; i < argc; ++i) {
            msg = msg.append(string(argv[i]));
        }
        
        msgSize = to_string(msg.size());
        spaces = "          ";
        msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
        client.sendMsg(&msgSize[0], DEFAULT_SIZE);
        client.sendMsg(&msg[0], msg.size());

        // Receiving the status message from the server:
        char buf[2048];
        client.recvMsg(buf, DEFAULT_SIZE);
        msg = string(buf);
        if (msg.compare(SUCCESS) == 0) {
            client.close();
            return 0;
        }
    } else {
        vector<thread> threads;
        for (int i = SP_INDEX + 1; i < argc; ++i) {
            threads.push_back(thread(subscribe, host, port, i, argv));
        }
        auto current = threads.begin();
        while (current != threads.end()) {
            current->join();
            ++current;
        }
    }

    return 1;
}

int main(int argc, char *argv[]) {

    int action;

    if (argc < 2)
        throwUsageError();
    string host = string(argv[HOST_INDEX]);
    string port = string(argv[PORT_INDEX]);
    string inputCommand = string(argv[SP_INDEX]);

    if (inputCommand.compare("subscribe") == 0)
        action = SUB;
    else if (inputCommand.compare("publish") == 0)
        action = PUB;
    else {
        cerr << "'" << inputCommand << "' not recognised" << endl;
        throwUsageError();
    }

    if (action == PUB && argc < PUB_ARGS
        || action == SUB && argc < SUB_ARGS)
        throwUsageError();
    
    if (sendRequest(host, port, action, argc, argv) == 0) {
        status = true;
        cout << "Your message got published!" << endl;
    }

    return 0;
}