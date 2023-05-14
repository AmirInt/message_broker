#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <condition_variable>
#include <chrono>
#include "server.hpp"


#ifdef _PLATFORM_WINDOWS

using namespace std;

mutex msgLock;
map<string, vector<string>> allMessages;
condition_variable cv;

void pingCurrentClient(Server server, SOCKET client, mutex& internalLock, int& unrespondedPings) {
    while (unrespondedPings < 3) {
        Sleep(MAX_WAITING_TIME * 1000 - 20);
        internalLock.lock();
        server.pingClient(client);
        Sleep(20);
        int response = server.getPong(client);
        switch (response) {
            case -1:
                cout << "Something went wrong" << endl;
                break;
            case 0:
                unrespondedPings = 0;
                break;
            case 1:
                ++unrespondedPings;
                cout << server.getClientName(client) << " with " << server.getClientIP(client) << ", "
                    << server.getClientPort(client) << " did not pong" << endl;
                break;
            case 2:
                unrespondedPings = 3;
                cout << server.getClientName(client) << " with " << server.getClientIP(client) << ", "
                    << server.getClientPort(client) << " closed connection" << endl;
        }
        internalLock.unlock();
    }
    server.deleteClient(client);
}

void handleSubscriber(Server server, SOCKET newClient, thread& currentThread) {
    
    char buf[BUFFER_SIZE];
    int msgSize, lastMsgSent = 0, unrespondedPings = 0;
    string msg, topic, size, messagesNumber;
    mutex internalLock;

    thread p(pingCurrentClient, server, newClient, ref(internalLock), ref(unrespondedPings));

    // Getting the topic:
    int result = server.recvMsg(newClient, buf);
    topic = string(buf);

    // Sending subscription acknowledgement to the client:
    result = server.sendMsg(newClient, SUCCESS);

    // Getting and sending the number of messages in requested topic:
    msgLock.lock();
    
    vector<string> topicMessages;
    
    topicMessages = allMessages[topic];
    lastMsgSent = topicMessages.size();
    
    msgLock.unlock();

    while (true) {
        mutex m;
        unique_lock<mutex> eventLock(m);    

        cv.wait(eventLock);
        if (unrespondedPings == 3)
            break;
        msgLock.lock();
        topicMessages = allMessages[topic];
        for (int i = lastMsgSent; i < topicMessages.size(); ++i) {
            internalLock.lock();
            server.sendMsg(newClient, MESSAGE);
            server.sendMsg(newClient, &topicMessages.at(i)[0]);
            internalLock.unlock();
        }
        lastMsgSent = topicMessages.size();
        msgLock.unlock();
    }
    p.join();
}

void handlePublisher(Server server, SOCKET newClient, thread& currentThread) {

    char buf[BUFFER_SIZE];
    int msgSize, lastMsgSent = 0, unrespondedPings = 0;
    string msg, topic, size, messagesNumber;
    mutex internalLock;

    // Creating a thread to ping the client over 10s periods:
    thread p(pingCurrentClient, server, newClient, ref(internalLock), ref(unrespondedPings));
    
    // Getting the topic of the message:
    server.recvMsg(newClient, buf);
    topic = string(buf);

    // Getting the message itself:
    server.recvMsg(newClient, buf);
    msg = string(buf);

    // Putting the new message into the storage:
    msgLock.lock();

    allMessages[topic].push_back(msg);

    msgLock.unlock();

    cv.notify_all();

    // Sending Acknowledgement:
    server.sendMsg(newClient, SUCCESS);

    p.join();
}

void handleClient(Server server, SOCKET newClient) {
    
    // Getting the first message from the client into 'buf'
    char buf[BUFFER_SIZE];
    string msg;

    server.recvMsg(newClient, buf);
    msg = string(buf);
    
    // Creating a thread based on the type of the client 'subscriber/publisher'
    thread t;
    if (msg.compare(SUB_SIGNAL) == 0)
        t = thread(handleSubscriber, server, newClient, ref(t));
    else if (msg.compare(PUB_SIGNAL) == 0)
        t = thread(handlePublisher, server, newClient, ref(t));
    t.detach();
}

int main() {

    // Creating a unique server to respond to incoming requests:
    Server server;

    // newClient receives and stores the clients sockets:
    SOCKET newClient;
    
    while (true) {
        // listening for and accepting incoming requests:
        server.welcomeClient(&newClient);
        cout << server.getClientName(newClient) << " connected" << endl;
        handleClient(server, newClient);
    }
}

#endif // _PLATFORM_WINDOWS

#ifdef _PLATFORM_LINUX

namespace server
{

Server::Server(uint port, int n_sockets_to_queue)
    : main_socket_(AF_INET, SOCK_STREAM, port)
    , main_tunnel_guardian_(std::thread(
        watchMainTunnel
    ))
{
    main_socket_.bindSocket();
    main_socket_.listenSocket(n_sockets_to_queue);
    welcomeCilents();
}

Server::~Server()
{
    main_socket_.closeSocket();
}

void Server::welcomeCilents()
{
    while (true) {
        int new_client_socket = main_socket_.acceptClient();
        client_handlers_.push_back(std::thread(
                handleClient
                , new_client_socket));
    }
}

void Server::watchMainTunnel()
{
    try {
        while (true) {
            if (not main_tunnel_.second.empty()) {
                distributePayload(main_tunnel_.second.front());
                main_tunnel_.second.pop();
            }
        }
    } catch (std::runtime_error&) {}
}

void Server::distributePayload(const Payload& payload)
{
    try {
        subscribing_clients_locks_[payload.first].lock();
        for (const auto& subscribing_client : subscribing_clients_[payload.first]) {
            try {
                static const std::string spaces{ "          " };
                // Send the initialiser
                std::string init{ std::to_string(constants::msg_signal) };
                std::string init_size{ std::to_string(init.length()) };
                init_size += spaces.substr(0, constants::default_size - init_size.size());
                socket_interface::sendOnSocket(subscribing_client, init_size);
                socket_interface::sendOnSocket(subscribing_client, init);
                
                // First, send the topic
                std::string topic_size{ std::to_string(payload.first.size()) };
                topic_size += spaces.substr(0, constants::default_size - topic_size.size());
                socket_interface::sendOnSocket(subscribing_client, topic_size);
                socket_interface::sendOnSocket(subscribing_client, payload.first);

                // Then, send the message itself
                std::string message_size{ std::to_string(payload.second.size()) };
                message_size += spaces.substr(0, constants::default_size - message_size.size());
                socket_interface::sendOnSocket(subscribing_client, message_size);
                socket_interface::sendOnSocket(subscribing_client, payload.second);
            } catch (std::runtime_error&) {}
        }
    } catch (std::out_of_range&) {}
}

void Server::handleClient(int socket_fd)
{
    
}

void Server::insertIntoMainTunnel(const Payload& payload)
{
    main_tunnel_.first.lock();
    main_tunnel_.second.push(payload);
    main_tunnel_.first.unlock();
}

} // namespace server

#endif // _PLATFORM_LINUX