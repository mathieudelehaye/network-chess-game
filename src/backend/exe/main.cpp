#include <iostream>

#include "network/NetworkMode.hpp"
#include "network/Server.hpp"

using namespace std;

int main() {
    Server server(NetworkMode::TCP, "127.0.0.1", 2000);

    server.start();
    std::cout << "Server running…" << std::endl;

    std::cin.get();  // wait for Enter
    server.stop();

    return 0;

    return 0;
}