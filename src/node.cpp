#include "node.hpp"

namespace node
{

void Node::sendMsg(int socket_fd, const std::string& message)
{
        // First, send the initialiser message
        // To pad initialiser messages
        static const std::string spaces{ "          " };
        std::string message_size{ std::to_string(message.size()) };
        message_size += spaces.substr(0, constants::default_size - message_size.size());
        socket_interface::sendOnSocket(socket_fd, message_size);

        // Then, send the message itself
        socket_interface::sendOnSocket(socket_fd, message);
}

void Node::sendMsg(socket_interface::Socket socket, const std::string& message)
{
        // First, send the initialiser message
        // To pad initialiser messages
        static const std::string spaces{ "          " };
        std::string message_size{ std::to_string(message.size()) };
        message_size += spaces.substr(0, constants::default_size - message_size.size());
        socket.sendMessage(message_size);

        // Then, send the message itself
        socket.sendMessage(message);
}

void Node::recvMsg(int socket_fd, std::string& message)
{
    // First, read the incoming message size
    std::string msg_size_string{};
    socket_interface::recvOnSocket(socket_fd, msg_size_string, constants::default_size);
    std::size_t msg_size{ std::stoul(msg_size_string) };

    // Now, read the main message
    socket_interface::recvOnSocket(socket_fd, message, msg_size);

}

void recvMsg(socket_interface::Socket socket, std::string& message)
{
    // First, read the incoming message size
    std::string msg_size_string{};
    socket.recvMessage(msg_size_string, constants::default_size);
    std::size_t msg_size{ std::stoul(msg_size_string) };

    // Now, read the main message
    socket.recvMessage(message, msg_size);

}

} // namespace node