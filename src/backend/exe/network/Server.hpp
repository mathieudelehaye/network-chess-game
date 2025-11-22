#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "GameContext.hpp"
#include "NetworkMode.hpp"
#include "Session.hpp"
#include "TransportFactory.hpp"

class Server {
public:
    Server(NetworkMode mode, const std::string& ip, int port);

    void start();
    void stop();

private:
    void acceptLoop(std::stop_token st);
    void cleanupLoop(std::stop_token st);
    void handleSessionClosed(const std::string& session_id);
    void cleanupClosedSessions();

    void connectTCP(const std::string& ip, int port);
    void connectIPC();
    void setupSendCallbacks();

    void broadcastToAll(const std::string& message);
    void broadcastToOthers(const std::string& exclude_session_id, const std::string& message);
    void unicastTo(const std::string& session_id, const std::string& message);

    int server_fd = -1;
    NetworkMode network;
    std::atomic<bool> running{false};

    std::jthread acceptThread;
    std::jthread cleanupThread;

    std::unordered_map<std::string, std::shared_ptr<Session>> sessions;
    std::mutex sessions_mutex_;

    std::vector<std::string> sessions_to_cleanup_;
    std::mutex cleanup_mutex_;

    /// All player sessions handled by the server share the same game controller
    /// (which includes the common game context shared by all players).
    std::shared_ptr<GameController> shared_controller_;
};
