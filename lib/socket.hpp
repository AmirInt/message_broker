// C++
#include <iostream>

#ifdef _PLATFORM_WINDOWS

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

namespace socket_interface
{

/**
 * \class Socket
 * \brief The Socket class provides an interface to use the
 * socket-related functions of the Windows socket library.
*/
class Socket {
    public:
        Socket();

        ~Socket();

        SOCKET getSocket(int af, int type, int protocol);

        void getBinded(SOCKET s, const sockaddr *addr, int addrlen);

        void startListening(SOCKET s, int backlog);

        int acceptClient(SOCKET s, sockaddr *addr, int *addrlen);

        void getConnected(SOCKET s, const sockaddr *name, int namelen);

        int sendMessage(SOCKET s, const char *buf, int len, int flags);

        // Terminates the programme if the peer closes the connection.
        int recvMessage(SOCKET s, char *buf, int len, int flags);

        void closeSocket(SOCKET s);

        void getAddrInfo(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA *pHints, ADDRINFOA **ppResult);

        int switchMode(SOCKET s, u_long *mode);

    private:
        WSADATA wsaData;
};

}

#endif

#define _PLATFORM_LINUX

#ifdef _PLATFORM_LINUX

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


namespace socket_interface
{

/**
 * \class Socket
 * \brief The Socket class provides an interface to use the
 * socket-related functions of the Windows socket library.
*/
class Socket {
    public:
        Socket(int network_protocol, __socket_type protocol, uint port);

        ~Socket();

        void bind_socket();

        void listen_socket(int n_sockets_to_queue);

        int accept_client();

        void connect_socket(const std::string& server_address);

        int send_message(const char *buf, int len, int flags);

        // Terminates the programme if the peer closes the connection.
        int recv_message(char *buf, int len, int flags);

        void close();

        // void get_addr_info(PCSTR pNodeName, PCSTR pServiceName, const ADDRINFOA *pHints, ADDRINFOA **ppResult);

        // int switch_mode(SOCKET s, u_long *mode);

    private:
        // Socket file descriptor
        int network_protocol_;
        __socket_type transport_protocol_;
        uint port_;
        int socket_fd_{};
        struct sockaddr_in address_;
        int opt_{ 1 };
        int addrlen_{ sizeof(address_) };
};

} // namespace socket_interface
#endif
