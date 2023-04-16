#include <string>
#include <map>
#include "socket.h"
#include "constants.h"

using namespace std;

/*!
    \class Server

    \brief The Server class aims to perform the server side
    functionalities of the programme.
*/
class Server {
    private:
        Socket socket;
        SOCKET s;
        int port;
        struct sockaddr_in addr;
        int addrlen;
        int lastClient;
        map<SOCKET, struct sockaddr_in> clients;
        map<SOCKET, string> clientNames;
        
        void start() {
            s = socket.getSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            socket.getBinded(s, (struct sockaddr *) &addr, addrlen);
            socket.startListening(s, SOMAXCONN);
        }

    public:
        /*!
            \fn Server &Server::Server()

            Constructs the server and returns it.
        */
        Server() {
            port = 9999;
    
            addrlen = sizeof(addr);
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);

            lastClient = 0;

            start();
        }

        /*!
            \fn void &Server::welcomeClient(SOCKET *newClient)

            Accepts a new client into newClient given.
        */
        void welcomeClient(SOCKET *newClient) {
            struct sockaddr_in clientInfo = {0};
            *newClient = socket.acceptClient(s, (struct sockaddr *) &clientInfo, &addrlen);
            // clients.insert(pair<SOCKET, struct sockaddr_in>(*newClient, clientInfo));
            clients[*newClient] = clientInfo;
            clientNames[*newClient] = "Client " + to_string(lastClient++);
        }

        /*!
            \fn char* &Server::getClientIP(SOCKET client)

            Returns the IP address of the given client socket.
        */
        char* getClientIP(SOCKET client) {
            return inet_ntoa(clients.at(client).sin_addr);
        }

        /*!
            \fn u_short &Server::getClientPort(SOCKET client)

            Returns the port of the given client socket.
        */
        u_short getClientPort(SOCKET client) {
            return clients.at(client).sin_port;
        }

        /*!
            \fn server &Server::getClientName(SOCKET client)

            Returns the stored name for the given client socket.
        */
        string getClientName(SOCKET client) {
            return clientNames.at(client);
        }

        /*!
            \fn void &Server::deleteClient(SOCKET client)

            Deletes the record for the given client socket.
        */
        void deleteClient(SOCKET client) {
            clients.erase(client);
            clientNames.erase(client);
        }

        /*!
            \fn int &Server::sendMsg(SOCKET client, const char *buf)

            Sends the given message inside the buffer to the given client.
        */
        int sendMsg(SOCKET client, const char *buf) {
            string spaces = "          ";
            string msg = string(buf); 
            string msgSize = to_string(msg.size());
            msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
            int bytesSent = socket.sendMessage(client, &msgSize[0], DEFAULT_SIZE, 0);
            bytesSent = socket.sendMessage(client, buf, msg.size(), 0);
            return bytesSent;
        }

        /*!
            \fn int &Server::sendMsg(SOCKET client, const char *buf)

            Receives a given message into the buffer from the given client.
        */
        int recvMsg(SOCKET client, char *buf) {
            int bytesRead = socket.recvMessage(client, buf, DEFAULT_SIZE, 0);
            buf[bytesRead] = '\0';
            int msgSize = atoi(buf);
            bytesRead = socket.recvMessage(client, buf, msgSize, 0);
            buf[bytesRead] = '\0';
            return bytesRead;
        }

        /*!
            \fn int &Server::pingClient(SOCKET client)

            Pings the given client.
        */
        int pingClient(SOCKET client) {
            return sendMsg(client, PING_MSG);
        }

        /*!
            \fn int &Server::getPong(SOCKET client)

            Gets the pong message from the given client.
        */
        int getPong(SOCKET client) {
            u_long mode = 1;
            int response = 1, readBytes = 0;
            char buf[BUFFER_SIZE];
            if (socket.switchMode(client, &mode) == 1)
                return -1;
            buf[0] = '\0';
            readBytes = socket.recvMessage(client, buf, DEFAULT_SIZE, 0);
            if (readBytes == WSAECONNRESET)
                response = 2;
            else if (readBytes != WSAEWOULDBLOCK)
                response = string(buf).compare(PONG_MSG);
            
            mode = 0;
            if (socket.switchMode(client, &mode) == 1)
                return -1;
            
            return response;
        }

        /*!
            \fn int &Server::close()

            Closes the server socket.
        */
        void close() {
            socket.closeSocket(s);
        }

        /*!
            \fn int &Server::close(SOCKET client)

            Closes the client socket.
        */
        void close(SOCKET client) {
            socket.closeSocket(client);
        }
};
