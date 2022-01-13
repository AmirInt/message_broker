#include <iostream>
#include <winsock2.h>
#include "socket.h"

class Server {
    private:
        Socket socket;
        SOCKET s;
        int port;
        struct sockaddr_in addr;
        int addrlen;

    public:
        Server() {
            port = 9999;
    
            addrlen = sizeof(addr);
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);
        }

        void start() {
            s = socket.getSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            
            socket.getBinded(s, (struct sockaddr *) &addr, addrlen);

            socket.startListening(s, SOMAXCONN);

            SOCKET newS;
            int newAddrlen;
            SOCKET newClient = socket.acceptClient(s, (struct sockaddr *) &addr, &newAddrlen);

            socket.closeSocket(s);

            char buf[11];
            int bytesRead = socket.recvMessage(newClient, buf, 10, 0);
            std::cout << "Server read: " << buf << " " << bytesRead << std::endl;
            socket.closeSocket(newClient);
        }
};