#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <condition_variable>
#include <chrono>
#include "server.h"
#include "constants.h"

using namespace std;

mutex msgLock;
map<string, vector<string>> allMessages;
condition_variable cv;


void handleClient(Server server, SOCKET newClient) {
    
    char buf[BUFFER_SIZE];
    int msgSize, lastMsgSent = 0, unrespondedPings = 0;
    string msg, topic, size, messagesNumber;

    server.recvMsg(newClient, buf);
    msg = string(buf);

    if (msg.compare(SUB_SIGNAL) == 0) {
        // Getting the topic:
        server.recvMsg(newClient, buf);
        topic = string(buf);

        // Sending subscription acknowledgement to the client:

        server.sendMsg(newClient, SUCCESS);

        // Getting and sending the number of messages in requested topic:

        msgLock.lock();

        map<string, vector<string>>::iterator it;
        vector<string> topicMessages;
        try {
            topicMessages = allMessages.at(topic);
            lastMsgSent = topicMessages.size();
            messagesNumber = to_string(topicMessages.size());
            server.sendMsg(newClient, &messagesNumber[0]);
            for (auto & elem: topicMessages) {
                server.sendMsg(newClient, &elem[0]);
            }
        } catch (const out_of_range& e) {
            server.sendMsg(newClient, "0         ");
        }

        msgLock.unlock();

        while (true) {
            mutex m;
            unique_lock<mutex> eventLock(m);
            if (cv.wait_for(eventLock, chrono::seconds(10)) == cv_status::timeout) {
                
            } else {
                msgLock.lock();
                topicMessages = allMessages[topic];
                for (int i = lastMsgSent; i < topicMessages.size(); ++i) {
                    server.sendMsg(newClient, MESSAGE);
                    server.sendMsg(newClient, &topicMessages.at(i)[0]);
                }
                lastMsgSent = topicMessages.size();
                msgLock.unlock();
            }
        }

    }
    else if (msg.compare(PUB_SIGNAL) == 0) {
        // Getting the topic of the message:
        server.recvMsg(newClient, buf);
        topic = string(buf);

        // Getting the message itself:
        server.recvMsg(newClient, buf);
        msg = string(buf);

        msgLock.lock();

        allMessages[topic].push_back(msg);
        cv.notify_all();
        
        msgLock.unlock();


        // Sending Acknowledgement:
        server.sendMsg(newClient, SUCCESS);
    }
}

int main() {

    Server server;
    
    SOCKET newClient;
    vector<thread> threads;
    while (true) {
        server.welcomeClient(&newClient);
        threads.push_back(thread(handleClient, server, newClient));
    }
    server.close();
}