cmake_minimum_required(VERSION 3.8)

project(message_broker)

add_compile_definitions(_PLATFORM_LINUX)

add_library(socket lib/socket.cpp)
target_include_directories(socket PRIVATE lib)

add_library(node src/node.cpp)
target_link_libraries(node socket)
target_include_directories(node PRIVATE lib)

add_library(client src/client.cpp)
target_link_libraries(client socket node)
target_include_directories(client PUBLIC lib)

add_library(server src/server.cpp)
target_link_libraries(server socket node)
target_include_directories(server PUBLIC lib)

add_executable(client_main src/client_main.cpp)
target_link_libraries(client_main client)

add_executable(server_main src/server_main.cpp)
target_link_libraries(server_main server)
