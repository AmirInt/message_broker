#include "socket.hpp"


#ifdef _PLATFORM_WINDOWS

namespace socket_interface
{

Socket::Socket()
{
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        std::cerr << "Initialisation failed" << std::endl;
}

Socket::~Socket()
{

}

SOCKET Socket::getSocket(int af, int type, int protocol)
{
    SOCKET socketDsc = socket(af, type, protocol);
    if (socketDsc == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
        WSACleanup();
        exit(1);
    }
    return socketDsc;
}


void getBinded(SOCKET s, const sockaddr *addr, int addrlen)
{
    if (bind(s, addr, addrlen) == SOCKET_ERROR) {
        std::cerr << "Socket binding failed" << std::endl;
        closesocket(s);
        WSACleanup();
        exit(1);
    }
}

void startListening(SOCKET s, int backlog)
{
    if (listen(s, backlog) == SOCKET_ERROR) {
        std::cerr << "Listening failed" << std::endl;
        closesocket(s);
        WSACleanup();
        exit(1);
    }
}

int acceptClient(SOCKET s, sockaddr *addr, int *addrlen)
{
    int newSocketDsc = accept(s, addr, addrlen);
    if (newSocketDsc == INVALID_SOCKET) {
        std::cerr << "Accepting client failed" << std::endl;
        closesocket(s);
        WSACleanup();
        exit(1);
    }
    return newSocketDsc;
}

void getConnected(SOCKET s, const sockaddr *name, int namelen)
{
    if (connect(s, name, namelen) == SOCKET_ERROR) {
        std::cerr << "Could not connect to the server" << std::endl;
        exit(1);
    }
}

int sendMessage(SOCKET s, const char *buf, int len, int flags)
{
    int sentBytesLen = send(s, buf, len, flags);
    if (sentBytesLen == SOCKET_ERROR)
        return -1;
    return sentBytesLen;
}

int recvMessage(SOCKET s, char *buf, int len, int flags)
{
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

void closeSocket(SOCKET s)
{
    if (closesocket(s) == SOCKET_ERROR) {
        std::cerr << "Closing connection failed" << std::endl;
        WSACleanup();
        exit(1);
    }
    WSACleanup();
}

void getAddrInfo(
    PCSTR pNodeName
    , PCSTR pServiceName
    , const ADDRINFOA *pHints
    , ADDRINFOA **ppResult)
{
    if (getaddrinfo(pNodeName, pServiceName, pHints, ppResult) != 0) {
        std::cerr << "Translation failed" << std::endl;
        WSACleanup();
        exit(1);
    }
}

int switchMode(SOCKET s, u_long *mode)
{
    if (ioctlsocket(s, FIONBIO, mode) == SOCKET_ERROR) {
        std::cerr << "Switching to new mode failed" << std::endl;
        closesocket(s);
        WSACleanup();
        return 1;
    }
    return 0;
}

} // namespace socket_interface

#endif

#ifdef _PLATFORM_LINUX

namespace socket_interface
{

Socket::Socket(int network_protocol, __socket_type transport_protocol, uint port)
    : network_protocol_(network_protocol)
    , transport_protocol_(transport_protocol)
    , port_(port)
{
    if (network_protocol_ < 0 or network_protocol_ > 45) // Check for validity of the network protocol.
        throw std::invalid_argument("Error: Incorrect network_protocol range. Exiting...");

    address_.sin_family = network_protocol_;
    address_.sin_addr.s_addr = INADDR_ANY;
    address_.sin_port = htons(port_);
    
    if ((socket_fd_ = socket(network_protocol_, transport_protocol_, 0)) < 0)
        throw std::runtime_error("Error: Creating socket failed. Exiting...");
}

Socket::~Socket()
{
    closeSocket();
}

void Socket::bindSocket()
{
    if (setsockopt(
            socket_fd_
            , SOL_SOCKET
            , SO_REUSEADDR | SO_REUSEPORT
            , &opt_
            , sizeof(opt_)) < 0) {
        std::cerr << "SetSockOpt failed. Exiting...\n";
        exit(EXIT_FAILURE);
    }

    if (bind(
            socket_fd_
            , static_cast<sockaddr*>(static_cast<void*>(&address_))
            , addrlen_) < 0) {
        std::cerr << "Binding socket failed. Exiting...\n";
        exit(EXIT_FAILURE);
    }
}

void Socket::listenSocket(int n_sockets_to_queue)
{
    if (listen(socket_fd_, n_sockets_to_queue) < 0) {
        std::cerr << "Listening failed. Exiting...\n";
        exit(EXIT_FAILURE);
    }
}

int Socket::acceptClient()
{
    int new_socket{};
    if ((new_socket = accept(
            socket_fd_
            , static_cast<sockaddr*>(static_cast<void*>(&address_))
            , static_cast<socklen_t*>(static_cast<void*>(&addrlen_)))) < 0) {
        std::cerr << "Accepting new connection failed. Exiting...\n";
        exit(EXIT_FAILURE);
    }

    return new_socket;
}

void Socket::connectSocket(const std::string& server_address)
{
    if (inet_pton(network_protocol_, server_address.c_str(), &address_.sin_addr) <= 0) {
        std::cerr << "Invalid address to connect to. Exiting...\n";
        exit(EXIT_FAILURE);
    }
    if (connect(
            socket_fd_
            , static_cast<sockaddr*>(static_cast<void*>(&address_))
            , addrlen_) < 0) {
        std::cerr << "Connecting to server failed. Exiting...\n";
        exit(EXIT_FAILURE);
    }
}

void Socket::sendMessage(const std::string& message, int flags)
{
    if (send(socket_fd_, message.c_str(), message.length(), flags) == -1)
        throw std::runtime_error("Error: Socket could not send the message.");
}

void Socket::recvMessage(std::string& message, std::size_t len, int flags)
{
    char buffer[constants::buffer_size]{};
    long read_len{};

    for (std::size_t read_so_far{ 0 }; read_so_far < len;) {
        read_len = recv(socket_fd_, buffer, constants::buffer_size, flags);
        if (len == -1)
            throw std::runtime_error("Error: Socket could not receive the message.");
        
        message += std::string(buffer, read_len);
        read_so_far += read_len;
    }
}

void Socket::closeSocket()
{
    close(socket_fd_);
    shutdown(socket_fd_, SHUT_RDWR);
}

} // namespace socket_interface

#endif