#include "socket.h"

class Server {
    private:
        Socket socket;
        SOCKET s;
        int port;
        struct sockaddr_in addr;
        int addrlen;
        
        void start() {
            s = socket.getSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            socket.getBinded(s, (struct sockaddr *) &addr, addrlen);
            socket.startListening(s, SOMAXCONN);
        }

    public:
        Server() {
            port = 9999;
    
            addrlen = sizeof(addr);
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);

            start();
        }

        void welcomeClient(SOCKET *newClient) {
            *newClient = socket.acceptClient(s, (struct sockaddr *) &addr, &addrlen);
        }

        void sendMsg(SOCKET client, const char *buf, int size) {
            int bytesSent = socket.sendMessage(client, buf, size, 0);
            std::cout << "Server sent: " << buf << " " << bytesSent << std::endl;
        }

        void recvMsg(SOCKET client, char *buf, int size) {
            int bytesRead = socket.recvMessage(client, buf, size, 0);
            buf[size] = '\0';
        }

        void close() {
            socket.closeSocket(s);
        }

        void close(SOCKET client) {
            socket.closeSocket(client);
        }
};