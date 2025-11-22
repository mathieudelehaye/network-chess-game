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

void Server::start() {
    running = true;

    // Start accept thread
    acceptThread = std::jthread([this](std::stop_token st) { acceptLoop(st); });

    // Start cleanup thread
    cleanupThread = std::jthread([this](std::stop_token st) { cleanupLoop(st); });
}

void Server::stop() {
    running = false;

    // Request stop for both threads
    acceptThread.request_stop();
    cleanupThread.request_stop();

    // Shutdown all sessions
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        for (auto& s : sessions) {
            s->close();
        }
    }

    // Shutdown server socket
    if (server_fd >= 0) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        server_fd = -1;
    }
}

void Server::acceptLoop(std::stop_token st) {
    auto& logger = Logger::instance();

    while (!st.stop_requested() && running.load()) {
        int client_fd = accept(server_fd, nullptr, nullptr);

        if (client_fd < 0) {
            continue;
        }

        logger.debug("Client connected");

        // Create   a unique transport layer for this session
        auto transport = TransportFactory::create(client_fd, network);

        // Create a session with its own transport and the shared controller
        auto session = std::make_shared<Session>(std::move(transport), shared_controller_);

        // Set close callback
        session->setCloseCallback(
            [this](const std::string& session_id) { this->handleSessionClosed(session_id); });

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

void Server::broadcastToAll(const std::string& message) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto& logger = Logger::instance();
    logger.debug("Broadcasting to all sessions: " + message);

    int count = 0;
    for (const auto& session : sessions) {
        // Skip if session is null or closed
        if (!session || !session->isActive()) {
            logger.trace("Skipping inactive session");
            continue;
        }
        session->send(message);
        count++;
    }

    logger.debug("Broadcast sent to " + std::to_string(count) + " sessions");
}

void Server::broadcastToOthers(const std::string& exclude_session_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);

    auto& logger = Logger::instance();
    logger.debug("Broadcasting to others (excluding " + exclude_session_id + "): " + message);

    int count = 0;
    for (const auto& session : sessions) {
        if (session->getSessionId() != exclude_session_id) {
            // Skip if session is null or closed
            if (!session || !session->isActive()) {
                logger.trace("Skipping inactive session");
                continue;
            }
            session->send(message);
            count++;
        }
    }

    logger.debug("Broadcast sent to " + std::to_string(count) + " sessions");
}

void Server::handleSessionClosed(const std::string& session_id) {
    auto& logger = Logger::instance();
    logger.debug("Handling session closed: " + session_id);

    // Queue for cleanup
    {
        std::lock_guard<std::mutex> lock(cleanup_mutex_);
        sessions_to_cleanup_.push_back(session_id);
    }

    // Notify game controller immediately
    shared_controller_->routeDisconnect(session_id);
}

void Server::cleanupLoop(std::stop_token st) {
    auto& logger = Logger::instance();
    logger.debug("Cleanup thread started");

    while (!st.stop_requested() && running.load()) {
        // Sleep for a time interval: delay can be extended since this is low
        // priority
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        // Process cleanup queue
        cleanupClosedSessions();
    }

    logger.debug("Cleanup thread exiting");
}

void Server::cleanupClosedSessions() {
    auto& logger = Logger::instance();

    // Get sessions to cleanup
    std::vector<std::string> to_cleanup;
    {
        std::lock_guard<std::mutex> lock(cleanup_mutex_);
        if (sessions_to_cleanup_.empty()) {
            return;  // Nothing to clean up
        }
        to_cleanup = std::move(sessions_to_cleanup_);
        sessions_to_cleanup_.clear();
    }

    logger.debug("Cleaning up " + std::to_string(to_cleanup.size()) + " sessions");

    // Remove sessions from the main list
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    for (const auto& session_id : to_cleanup) {
        auto it = std::find_if(sessions.begin(), sessions.end(),
                               [&session_id](const std::shared_ptr<Session>& s) {
                                   return s->getSessionId() == session_id;
                               });

        if (it != sessions.end()) {
            logger.debug("Removing session from list: " + session_id);
            sessions.erase(it);
        }
    }
}