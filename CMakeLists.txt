cmake_minimum_required(VERSION 3.8)

project(message_broker)

add_compile_definitions(_PLATFORM_LINUX)

add_library(socket lib/socket.cpp)
target_include_directories(socket PRIVATE lib)

add_library(client src/client.cpp)
target_link_libraries(client socket)
target_include_directories(client PRIVATE lib)