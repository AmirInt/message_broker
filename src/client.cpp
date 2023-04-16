#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <winsock2.h>
#include <windows.h>
#include "client.h"
#include "constants.h"

using namespace std;

bool status;
mutex outputLock;
condition_variable cv;

void throwUsageError() {
    cerr << endl << "Usage:\tpublish [Topic] [\"Message\"]" << endl
    << "      \tor"
    << endl << "      \tsubscribe [Topic 1] [Topic 2] ... [Topic N]" << endl;
    exit(0);
}

void subscribeTopic(string host, string port, int topicIndex, char *argv[]) {

    string msg, msgSize, spaces = "          ";
    char buf[BUFFER_SIZE];
    int messageSize, result;

    // Creating a client and sending a subscribe signal:
    Client client(&host[0], &port[0]);
    client.sendMsg(SUB_SIGNAL);

    // Sending the topic of the message:
    client.sendMsg(argv[topicIndex]);

    // Receiving the server's response:
    client.recvMsg(buf);
    outputLock.lock();
    if (string(buf).compare(SUCCESS) == 0) {
        status = true;
        cout << endl << "Subscribed successfully on '" << argv[topicIndex] << "'" << endl;
    }

    cv.notify_one();

    outputLock.unlock();

    // Waiting for new messages from the server:
    while (true) {
        result = client.recvMsg(buf);
        if (string(buf).compare(MESSAGE) == 0) {
            client.recvMsg(buf);
            outputLock.lock();
            cout << endl << "\tNew message on topic '" << argv[topicIndex] << "'" << endl;
            cout << "\t\t" << buf << endl << endl;
            outputLock.unlock();
        }
        else if (string(buf).compare(PING_MSG) == 0)
            client.pong();
        else
            break;
    }

    client.close();
}

void subscribe(string host, string port, int argc, char *argv[]) {
    vector<thread> threads;
    for (int i = SP_INDEX + 1; i < argc; ++i)
        threads.push_back(thread(subscribeTopic, host, port, i, argv));
    auto current = threads.begin();
    while (current != threads.end()) {
        current->join();
        ++current;
    }
}

void publish(string host, string port, int argc, char *argv[]) {
    
    string msg;
    
    // Creating a client and sending publish request:
    Client client(&host[0], &port[0]);
    client.sendMsg(PUB_SIGNAL);

    // Sending the topic of the message:
    client.sendMsg(argv[SP_INDEX + 1]);

    // Sending the message itself:
    msg = string(argv[SP_INDEX + 2]);
    for (int i = SP_INDEX + 3; i < argc; ++i)
        msg = msg.append(string(argv[i]));
    
    client.sendMsg(&msg[0]);

    // Receiving the status message from the server under 10 seconds:

    char buf[2048];

    // Waiting for ping messages from the server:
    while (true) {
        client.recvMsg(buf);
        if (string(buf).compare(SUCCESS) == 0) {
            status = true;
            cout << "Your message got published :)" << endl;
            cv.notify_one();
        }
        if (string(buf).compare(PING_MSG) == 0)
            client.pong();
    }

    client.close();
}

int main(int argc, char *argv[]) {

    int action;

    if (argc < SUB_ARGS)
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

    if (action == PUB && argc < PUB_ARGS)
        throwUsageError();

    status = false;
    thread t;

    if (action == SUB)
        t = thread(subscribe, host, port, argc, argv);
    else
        t = thread(publish, host, port, argc, argv);

    mutex m;
    unique_lock<mutex> eventLock(m);
    if (cv.wait_for(eventLock, chrono::seconds(MAX_WAITING_TIME)) == cv_status::timeout) {
        t.detach();
        cerr << "There was an issue processing your request" << endl;
        t.~thread();
    } else
        t.join();

    return 0;
}