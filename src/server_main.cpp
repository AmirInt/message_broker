// C++
#include <iostream>

// Package
#include "server.hpp"


void display_usage()
{
    std::cout << "Usage:\n";
    std::cout << "[./server_main | server_main.exe] <port>";
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        display_usage();
        return 1;
    }

    auto server{ server::Server(std::stoi(argv[1])) };
}