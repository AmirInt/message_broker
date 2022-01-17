#include <string>
#include "socket.h"
#include "constants.h"

using namespace std;

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
        Client(char *host, char *port) {
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            socket.getAddrInfo(host, port, &hints, &result);
            start();
        }

        void sendMsg(const char *buf) {
            string spaces = "          ";
            string msg = string(buf); 
            string msgSize = to_string(msg.size());
            msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
            int bytesSent = socket.sendMessage(s, &msgSize[0], DEFAULT_SIZE, 0);
            bytesSent = socket.sendMessage(s, buf, msg.size(), 0);
        }

        void recvMsg(char *buf) {
            int bytesRead = socket.recvMessage(s, buf, DEFAULT_SIZE, 0);
            buf[bytesRead] = '\0';
            int msgSize = atoi(buf);
            bytesRead = socket.recvMessage(s, buf, msgSize, 0);
            buf[bytesRead] = '\0';
        }

        void close() {
            socket.closeSocket(s);
        }
};