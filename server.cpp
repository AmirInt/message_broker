#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include "server.h"

#define SUB 0
#define PUB 1
#define SUB_ARGS 3
#define PUB_ARGS 4

#define SUB_SIGNAL "0000000231"
#define PUB_SIGNAL "0000000014"

#define SUCCESS "0000002512"

#define DEFAULT_SIZE 10
#define BUFFER_SIZE 2048

using namespace std;

mutex msgLock;
map<string, vector<string>> allMessages;

void handleClient(Server server, SOCKET newClient) {
    char buf[BUFFER_SIZE];
    server.recvMsg(newClient, buf, DEFAULT_SIZE);
    string msg = string(buf);
    if (msg.compare(SUB_SIGNAL) == 0) {
        // Getting the topic:
        server.recvMsg(newClient, buf, DEFAULT_SIZE);
        int msgSize = stoi(string(buf));
        server.recvMsg(newClient, buf, msgSize);
        string topic = string(buf);

        // Getting and sending the number of messages in requested topic:

        msgLock.lock();

        map<string, vector<string>>::iterator it;
        vector<string> topicMessages;
        for (it = allMessages.begin(); it != allMessages.end(); ++it)
            if (it->first.compare(topic) == 0) {
                topicMessages = it->second;
                break;
            }

        if (it == allMessages.end())
            server.sendMsg(newClient, "0         ", DEFAULT_SIZE);
        else {
            string messagesNumber = to_string(topicMessages.size());
            string spaces = "          ";
            messagesNumber = messagesNumber.append(spaces, 0, DEFAULT_SIZE - messagesNumber.size());
            server.sendMsg(newClient, &messagesNumber[0], DEFAULT_SIZE);
            for (auto & elem: topicMessages) {
                string size = to_string(elem.size());
                size = size.append(spaces, 0, DEFAULT_SIZE);
                server.sendMsg(newClient, &size[0], DEFAULT_SIZE);
                server.sendMsg(newClient, &elem[0], elem.size());
            }
        }

        msgLock.unlock();
    }
    else if (msg.compare(PUB_SIGNAL) == 0) {
        // Getting the topic of the message:
        server.recvMsg(newClient, buf, DEFAULT_SIZE);
        int msgSize = stoi(string(buf));
        server.recvMsg(newClient, buf, msgSize);
        string topic = string(buf);

        // Getting the message itself:
        server.recvMsg(newClient, buf, DEFAULT_SIZE);
        msgSize = stoi(string(buf));
        server.recvMsg(newClient, buf, msgSize);
        string msg = string(buf);

        msgLock.lock();

        map<string, vector<string>>::iterator it;
        for (it = allMessages.begin(); it != allMessages.end(); ++it) {
            if (it->first.compare(topic) == 0) {
                it->second.push_back(msg);
                break;
            }
        }

        if (it == allMessages.end()) {
            vector<string> newSubject;
            newSubject.push_back(msg);
            allMessages.insert(pair<string, vector<string>>(topic, newSubject));
        }

        msgLock.unlock();
        
        // Sending Acknowledgement:
        server.sendMsg(newClient, SUCCESS, DEFAULT_SIZE);
    }
}

int main() {

    string topic1 = "topic1";
    vector<string> vector1;
    vector1.push_back(string("Hello this is message 1 of 1"));
    vector1.push_back(string("Hello this is message 2 of 1"));

    allMessages.insert(pair<string, vector<string>>(topic1, vector1));

    string topic2 = "topic2";
    vector<string> vector2;
    vector2.push_back(string("Hello this is message 1 of 2"));
    vector2.push_back(string("Hello this is message 2 of 2"));

    allMessages.insert(pair<string, vector<string>>(topic2, vector2));

    Server server;
    
    SOCKET newClient;
    vector<thread> threads;
    while (true) {
        server.welcomeClient(&newClient);
        threads.push_back(thread(handleClient, server, newClient));
    }
    server.close();
}