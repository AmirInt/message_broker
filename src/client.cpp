#include "constants.hpp"
#include "client.hpp"


#ifdef _PLATFORM_WINDOWS

bool status;
std::mutex outputLock;
std::condition_variable cv;

void throwUsageError() {
    std::cerr << std::endl << "Usage:\tpublish [Topic] [\"Message\"]" << endl
    << "      \tor"
    << endl << "      \tsubscribe [Topic 1] [Topic 2] ... [Topic N]" << endl;
    exit(0);
}

void subscribeTopic(std::string host, std::string port, int topicIndex, char *argv[]) {

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

#endif // _PLATFORM_WINDOWS

#ifdef _PLATFORM_LINUX

namespace client
{
    Client::Client(const std::string& host, uint port)
        : socket_(AF_INET, SOCK_STREAM, port)
    {
        socket_.connectSocket(host);
    }

    Client::~Client()
    {
        close();
    }

    void Client::sendMsg(const std::string& message)
    {
        // First, send the initialiser message
        // To pad initialiser messages
        static const std::string spaces{ "          " };
        std::string message_size{ std::to_string(message.size()) };
        message_size += spaces.substr(0, constants::default_size - message_size.size());
        socket_.sendMessage(message_size);

        // Then, send the message itself
        socket_.sendMessage(message);
    }

    void Client::recvMsg(std::string& message)
    {
        // First, read the incoming message size
        std::string msg_size_string{};
        socket_.recvMessage(msg_size_string, constants::default_size);
        std::size_t msg_size{ std::stoul(msg_size_string) };

        // Now, read the main message
        socket_.recvMessage(message, msg_size);
    }

    void Client::subscribe(const std::string& topic)
    {
        sendMsg(std::to_string(constants::sub_signal));
        sendMsg(topic);
        
        std::string response;
        recvMsg(response);
        if (response == std::to_string(constants::success))
            std::cout << "Successfully subscribed to " << topic << '\n';
        else
            std::cerr << "Failed to subscribed to " << topic << '\n';
    }

    void Client::publish(const std::string& topic, const std::string& message)
    {
        sendMsg(std::to_string(constants::pub_signal));
        sendMsg(topic);
        sendMsg(message);

        std::string response;
        recvMsg(response);
        if (response == std::to_string(constants::success))
            std::cout << "Successfully published message.\n";
        else
            std::cerr << "Failed to publish message.";
    }

    void Client::pong()
    {
        sendMsg(std::to_string(constants::pong));
    }

    void Client::close()
    {
        socket_.closeSocket();
    }

}

#endif // _PLATFORM_LINUX