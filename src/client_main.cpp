// C++
#include <iostream>

// Package
#include "client.hpp"


void display_usage()
{
    std::cout << "Usage:\n";
    std::cout << "[./client_main | client_main.exe] <host> <port>";
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        display_usage();
        return 1;
    }

    auto client{ client::Client(argv[0], std::stoi(argv[1])) };
}