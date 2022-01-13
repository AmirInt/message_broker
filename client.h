#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include "socket.h"

class Client {
    private:
        Socket socket;
        SOCKET s;
        char port[5] = "9999";
        struct addrinfo *result = NULL, hints;

    public:
        Client() {
            ZeroMemory(&hints, sizeof(hints));
            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            socket.getAddrInfo("127.0.0.1", port, &hints, &result);
        }

        void start() {
            s = socket.getSocket(result->ai_family, result->ai_socktype, result->ai_protocol);
            
            socket.getConnected(s, result->ai_addr, (int) result->ai_addrlen);

            char *buf = "Why Aye Ma";

            int bytesSent = socket.sendMessage(s, buf, (int) strlen(buf), 0);
            
            socket.shutDown(s, SD_SEND);
            
            std::cout << "Client sent: " << buf << " " << bytesSent << std::endl;
            socket.closeSocket(s);
        }
};