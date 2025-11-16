#include "Server.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "utils/Logger.hpp"

Server::Server(NetworkMode mode, const std::string& ip, int port) : network(mode) {
    // Create socket based on transport mode
    switch (network) {
        case NetworkMode::IPC:
            connectIPC();
            break;
        case NetworkMode::TCP:
        default:
            // Select TCP by default
            connectTCP(ip, port);
            break;
    }
}

void Server::start() {
    running = true;

    acceptThread = std::jthread([this](std::stop_token st) { acceptLoop(st); });
}

void Server::stop() {
    running = false;

    acceptThread.request_stop();

    // Shutdown all sessions
    for (auto& s : sessions)
        s->close();

    if (server_fd >= 0)
        ::shutdown(server_fd, SHUT_RDWR);
}

void Server::acceptLoop(std::stop_token st) {
    auto& logger = Logger::instance();

    while (!st.stop_requested() && running.load()) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd < 0)
            continue;  // temporary error, try again

        logger.debug("Client connected");

        // Create a transport
        auto transport = TransportFactory::create(client_fd, network);

        // Create a session to own the transport with multiple clients
        auto session = std::make_shared<Session>(std::move(transport));

        sessions.push_back(session);

        // Start the session
        session->start();
    }
}

void Server::connectTCP(const std::string& ip, int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("Cannot create TCP socket");

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
        throw std::runtime_error("Invalid IP address");

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("TCP bind failed");

    if (listen(server_fd, 10) < 0)
        throw std::runtime_error("TCP listen failed");
}

void Server::connectIPC() {
    // TODO: implement
}
