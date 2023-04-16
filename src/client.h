#include <string>
#include "socket.h"
#include "constants.h"

using namespace std;

/*!
    \class Client

    \brief The Client class has functions to manipulate a client
    in the programme.
*/
class Client {
    private:
        Socket socket;
        SOCKET s;
        struct addrinfo *result = NULL, hints;

        void start() {
            s = socket.getSocket(result->ai_family, result->ai_socktype, result->ai_protocol);
            socket.getConnected(s, result->ai_addr, (int) result->ai_addrlen);
        }

    public:
        /*!
            \fn Client &Client::Client(char *host, char *port)

            Constructs the client and returns it.
        */
        Client(char *host, char *port) {
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            socket.getAddrInfo(host, port, &hints, &result);
            start();
        }

        /*!
            \fn int &Client::sendMsg(const char *buf)

            Sends the given message inside the buffer to the client's server.
        */
        int sendMsg(const char *buf) {
            string spaces = "          ";
            string msg = string(buf); 
            string msgSize = to_string(msg.size());
            msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
            int bytesSent = socket.sendMessage(s, &msgSize[0], DEFAULT_SIZE, 0);
            if (bytesSent == -1) {
                cout << "Could not send message to the server. Try again later" << endl;
                exit(1);
            }
            bytesSent = socket.sendMessage(s, buf, msg.size(), 0);
            if (bytesSent == -1) {
                cout << "Could not send message to the server. Try again later" << endl;
                exit(1);
            }
            return 0;
        }

        /*!
            \fn int &Client::recvMsg(char *buf)

            Receives a message from the client's server and puts it
            inside the buffer.
            This function makes one byte immediately following the 
            last character read a null character.
        */
        int recvMsg(char *buf) {
            int bytesRead = socket.recvMessage(s, buf, DEFAULT_SIZE, 0);
            buf[bytesRead] = '\0';
            int msgSize = atoi(buf);
            bytesRead = socket.recvMessage(s, buf, msgSize, 0);
            buf[bytesRead] = '\0';
            return bytesRead;
        }

        /*!
            \fn int &Client::pong()

            Sends the pong message to the server.
        */
        int pong() {
            return socket.sendMessage(s, PONG_MSG, DEFAULT_SIZE, 0);
        }

        /*!
            \fn int &Client::pong()

            Closes the connection to the client's server.
        */
        void close() {
            socket.closeSocket(s);
        }
};