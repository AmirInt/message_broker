#ifndef NODE_HPP_
#define NODE_HPP_

// C++
#include <string>

// Package
#include "constants.hpp"
#include "socket.hpp"

namespace node
{

class Node {
    protected:
        void sendMsg(int socket_fd, const std::string& message);
        
        void sendMsg(socket_interface::Socket socket, const std::string& message);

        void recvMsg(int socket_fd, std::string& message);

        void recvMsg(socket_interface::Socket socket, std::string& message);

};

} // namespace node


#endif // NODE_HPP_