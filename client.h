#include <string>
#include "socket.h"

class Client {
    private:
        Socket socket;
        SOCKET s;
        std::string port;
        struct addrinfo *result = NULL, hints;

        void start() {
            s = socket.getSocket(result->ai_family, result->ai_socktype, result->ai_protocol);
            socket.getConnected(s, result->ai_addr, (int) result->ai_addrlen);
        }

    public:
        Client() {
            port = "9999";
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            socket.getAddrInfo("127.0.0.1", &port[0], &hints, &result);
            start();
        }

        void sendMsg(const char *buf, int size) {
            int bytesSent = socket.sendMessage(s, buf, size, 0);
        }

        void recvMsg(char *buf, int size) {
            int bytesRead = socket.recvMessage(s, buf, size, 0);
            buf[size] = '\0';
        }

        void close() {
            socket.closeSocket(s);
        }
};