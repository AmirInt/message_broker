#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

/*!
    \class Socket

    \brief The Socket class provides an interface to use the
    socket-related functions of the Windows socket library.
*/
class Socket {

    private:
        WSADATA wsaData;

    public:
        Socket() {
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
                std::cerr << "Initialisation failed" << std::endl;
        }

        SOCKET getSocket(int af, int type, int protocol) {
            SOCKET socketDsc = socket(af, type, protocol);
            if (socketDsc == INVALID_SOCKET) {
                std::cerr << "Socket creation failed" << std::endl;
                WSACleanup();
                exit(1);
            }
            return socketDsc;
        }

        void getBinded(SOCKET s, const sockaddr *addr, int addrlen) {
            if (bind(s, addr, addrlen) == SOCKET_ERROR) {
                std::cerr << "Socket binding failed" << std::endl;
                closesocket(s);
                WSACleanup();
                exit(1);
            }
        }

        void startListening(SOCKET s, int backlog) {
            if (listen(s, backlog) == SOCKET_ERROR) {
                std::cerr << "Listening failed" << std::endl;
                closesocket(s);
                WSACleanup();
                exit(1);
            }
        }

        int acceptClient(SOCKET s, sockaddr *addr, int *addrlen) {
            int newSocketDsc = accept(s, addr, addrlen);
            if (newSocketDsc == INVALID_SOCKET) {
                std::cerr << "Accepting client failed" << std::endl;
                closesocket(s);
                WSACleanup();
                exit(1);
            }
            return newSocketDsc;
        }

        void getConnected(SOCKET s, const sockaddr *name, int namelen) {
            if (connect(s, name, namelen) == SOCKET_ERROR) {
                std::cerr << "Could not connect to the server" << std::endl;
                exit(1);
            }
        }

        int sendMessage(SOCKET s, const char *buf, int len, int flags) {
            int sentBytesLen = send(s, buf, len, flags);
            if (sentBytesLen == SOCKET_ERROR)
                return -1;
            return sentBytesLen;
        }

        // Terminates the programme if the peer closes the connection.
        int recvMessage(SOCKET s, char *buf, int len, int flags) {
            int recvedBytesLen = recv(s, buf, len, flags);
            int errorNo = WSAGetLastError();
            if (errorNo == WSAEWOULDBLOCK || errorNo == WSAECONNRESET)
                return errorNo;
            if (recvedBytesLen == SOCKET_ERROR) {
                std::cerr << "Reading failed" << std::endl;
                return -1;
            }
            return recvedBytesLen;
        }

        void closeSocket(SOCKET s) {
            if (closesocket(s) == SOCKET_ERROR) {
                std::cerr << "Closing connection failed" << std::endl;
                WSACleanup();
                exit(1);
            }
            WSACleanup();
        }

        void getAddrInfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA *pHints, ADDRINFOA **ppResult) {
            if (getaddrinfo(pNodeName, pServiceName, pHints, ppResult) != 0) {
                std::cerr << "Translation failed" << std::endl;
                WSACleanup();
                exit(1);
            }
        }

        int switchMode(SOCKET s, u_long *mode) {
            if (ioctlsocket(s, FIONBIO, mode) == SOCKET_ERROR) {
                std::cerr << "Switching to new mode failed" << std::endl;
                closesocket(s);
                WSACleanup();
                return 1;
            }
            return 0;
        }
};