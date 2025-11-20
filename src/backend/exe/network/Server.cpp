#include "Server.hpp"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <nlohmann/json.hpp>

#include "GameController.hpp"
#include "Logger.hpp"

using json = nlohmann::json;

Server::Server(NetworkMode mode, const std::string& ip, int port)
    : network(mode), shared_controller_(std::make_shared<GameController>()) {
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

    setupBroadcastCallback();
}

void Server::setupBroadcastCallback() {
    shared_controller_->setBroadcastCallback(
        [this](const std::string& originating_session_id, const json& msg, bool to_all) {
            auto& logger = Logger::instance();
            logger.debug("Server::setupBroadcastCallback");
            
            std::string message = msg.dump();
            logger.debug("Server::setupBroadcastCallback: message = " + message);

            if (to_all) {
                this->broadcastToAll(message);
            } else {
                this->broadcastToOthers(originating_session_id, message);
            }
        });
}

void Server::broadcastToAll(const std::string& message) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto& logger = Logger::instance();
    logger.debug("Broadcasting to all sessions: " + message);

    for (const auto& session : sessions) {
        session->send(message);
    }

    logger.debug("Broadcast sent to " + std::to_string(sessions.size()) + " sessions");
}

void Server::broadcastToOthers(const std::string& exclude_session_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto& logger = Logger::instance();
    logger.debug("Broadcasting to others (excluding " + exclude_session_id + "): " + message);

    int count = 0;
    for (const auto& session : sessions) {
        if (session->getSessionId() != exclude_session_id) {
            session->send(message);
            count++;
        }
    }

    logger.debug("Broadcast sent to " + std::to_string(count) + " sessions");
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
        shutdown(server_fd, SHUT_RDWR);
}

void Server::acceptLoop(std::stop_token st) {
    auto& logger = Logger::instance();

    while (!st.stop_requested() && running.load()) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        // If no connection request, we use that time to clean up sessions
        if (client_fd < 0) {
            cleanupDeadSessions();
            continue;
        }

        logger.debug("Client connected");

        // Create a unique transport layer for this session
        auto transport = TransportFactory::create(client_fd, network);

        // Create a session with its own transport and the shared controller
        auto session = std::make_shared<Session>(std::move(transport), shared_controller_);

        {
            // Add the session to the list of active sessions (thread-safe)
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            sessions.push_back(session);
        }

        // Start the session (e.g., begin receiving messages)
        session->start();
    }
}

void Server::connectTCP(const std::string& ip, int port) {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
        throw std::runtime_error("Cannot create TCP socket");

    // Enable SO_REUSEADDR to allow immediate reuse of the port
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to set SO_REUSEADDR: " + std::string(strerror(errno)));
    }

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

void Server::cleanupDeadSessions() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto& logger = Logger::instance();

    // Remove sessions that are no longer active
    auto it = sessions.begin();
    while (it != sessions.end()) {
        if (!(*it)->isActive()) {
            logger.debug("Removing dead session: " + (*it)->getSessionId());
            it = sessions.erase(it);
        } else {
            ++it;
        }
    }
}