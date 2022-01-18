#include <string>
#include "socket.h"
#include "constants.h"

using namespace std;

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

        int sendMsg(SOCKET client, const char *buf) {
            string spaces = "          ";
            string msg = string(buf); 
            string msgSize = to_string(msg.size());
            msgSize = msgSize.append(spaces, 0, DEFAULT_SIZE - msgSize.size());
            int bytesSent = socket.sendMessage(client, &msgSize[0], DEFAULT_SIZE, 0);
            bytesSent = socket.sendMessage(client, buf, msg.size(), 0);
            return bytesSent;
        }

        int recvMsg(SOCKET client, char *buf) {
            int bytesRead = socket.recvMessage(client, buf, DEFAULT_SIZE, 0);
            buf[bytesRead] = '\0';
            int msgSize = atoi(buf);
            bytesRead = socket.recvMessage(client, buf, msgSize, 0);
            buf[bytesRead] = '\0';
            return bytesRead;
        }

        int pingClient(SOCKET client) {
            return sendMsg(client, PING_MSG);
        }

        int getPong(SOCKET client) {
            u_long mode = 1;
            int response = 1, readBytes = 0;
            char buf[BUFFER_SIZE];
            if (socket.switchMode(client, &mode) == 1)
                return 1;
            buf[0] = '\0';
            readBytes = socket.recvMessage(client, buf, DEFAULT_SIZE, 0);
            if (readBytes == WSAECONNRESET)
                response = 2;
            else if (readBytes != WSAEWOULDBLOCK)
                response = string(buf).compare(PONG_MSG);
            
            mode = 0;
            if (socket.switchMode(client, &mode) == 1)
                return 1;
            
            return response;
        }

        void close() {
            socket.closeSocket(s);
        }

        void close(SOCKET client) {
            socket.closeSocket(client);
        }
};
