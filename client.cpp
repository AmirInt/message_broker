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

    string msg, msgSize, spaces = "          ";
    char buf[BUFFER_SIZE];
    int messagesNumber, messageSize;

    // Creating a client and sending a subscribe signal:
    Client client(&host[0], &port[0]);
    client.sendMsg(SUB_SIGNAL);

    // Sending the topic of the message:
    client.sendMsg(argv[topicIndex]);

    // Receiving the server's response:
    client.recvMsg(buf);
    outputLock.lock();
    if (string(buf).compare(SUCCESS) == 0)
        cout << endl << "Subscribed successfully on '" << argv[topicIndex] << "'" << endl;

    // Receiving the number of messages from the server:
    client.recvMsg(buf);
    messagesNumber = atoi(buf);
    cout << endl;
    if (messagesNumber == 0)
        cout << "\tYou have no messages on topic '" << argv[topicIndex] << "':" << endl;
    else {
        cout << "\tMessages on topic '" << argv[topicIndex] << "':" << endl;
        for (int i = 0; i < messagesNumber; ++i) {
            client.recvMsg(buf);
            cout << "\t\t" << i << ". " << buf << endl;
        }
    }
    outputLock.unlock();

    // Waiting for new messages from the server:
    while (true) {
        client.recvMsg(buf);
        if (string(buf).compare(MESSAGE) == 0) {
            client.recvMsg(buf);
            outputLock.lock();
            cout << endl << "New message on topic '" << argv[topicIndex] << "'" << endl;
            cout << "\t" << buf << endl << endl;
            outputLock.unlock();
        }
        else if (string(buf).compare(PING_MSG) == 0) {

        }
    }

    client.close();
}

void sendRequest(string host, string port, int action, int argc, char *argv[]) {
    
    string msg;

    if (action == PUB) {
        // Creating a client and sending publish request:
        Client client(&host[0], &port[0]);
        client.sendMsg(PUB_SIGNAL);

        // Sending the topic of the message:
        client.sendMsg(argv[SP_INDEX + 1]);

        // Sending the message itself:
        msg = string(argv[SP_INDEX + 2]);
        for (int i = SP_INDEX + 3; i < argc; ++i) {
            msg = msg.append(string(argv[i]));
        }
        
        client.sendMsg(&msg[0]);

        // Receiving the status message from the server:
        char buf[2048];
        client.recvMsg(buf);
        msg = string(buf);
        if (msg.compare(SUCCESS) == 0) {
            cout << "Your message got published!" << endl;
            client.close();
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
    
    sendRequest(host, port, action, argc, argv);
    
    return 0;
}