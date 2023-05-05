#ifndef CONSTANTS_HPP_
#define CONSTANTS_HPP_

#include <string>

/**
 * \brief Determines the type of the created client
*/
enum class ClientType
{
    sub,
    pub,
};

/**
 * \brief Determines the type of the message to send
*/
enum class MessageType
{
    ping,
    pong,
    user,
};

namespace constants
{
    // The number of the passed arguments to a client of type subscriber
    constexpr int sub_args_count{ 5 };
    // The number of the passed arguments to a client of type publisher
    constexpr int pub_args_count{ 6 };

    // The index of the host address among the passed programme arguments
    constexpr int host_index{ 1 };
    // The index of the port number among the passed programme arguments
    constexpr int port_index{ 1 };
    // The index of the data among the passed programme arguments
    constexpr int data_index{ 1 };

    // The default initialiser message size
    constexpr int default_size{ 10 };
    // The size of the buffer
    constexpr int buffer_size{ 2048 };
    // The maximum time the client waits for the server to respond
    constexpr int max_waiting_time{ 10 }; // seconds

    // The subscribe and publish signals
    constexpr int sub_signal{ 1001 };
    constexpr int pub_signal{ 1002 };

    // The success and failure signals
    constexpr int success{ 200 };
    constexpr int failure{ 400 };

}

#endif