#ifndef LIB__CONSTANTS_HPP_
#define LIB__CONSTANTS_HPP_


/**
 * \brief Determines the type of the created client
*/
enum class ClientType
{
    sub,
    pub,
};

/**
 * \brief Determines the type of the signal.
*/
enum class SignalType
{
    // Determines that the client wants to subscribe to a topic
    sub_signal,
    // Determines that the client wants to publish a message
    pub_signal,
};

/**
 * \brief Determines the status of the most recently sent message
*/
enum class Status
{
    success,
    failure,
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
    constexpr int sub_args_count{ 6 };

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
}

#endif