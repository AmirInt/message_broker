cmake_minimum_required(VERSION 3.8)

project(message_broker)

set(_PLATFORM_LINUX)

add_library(socket lib/socket.cpp)
target_include_directories(socket PRIVATE lib)