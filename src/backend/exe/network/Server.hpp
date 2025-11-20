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

    void broadcastToAll(const std::string& message);
    void broadcastToOthers(const std::string& exclude_session_id, const std::string& message);

   private:
    void acceptLoop(std::stop_token st);
    void connectTCP(const std::string& ip, int port);
    void connectIPC();
    void setupBroadcastCallback();
    void cleanupDeadSessions();

    NetworkMode network;
    std::jthread acceptThread;
    std::atomic<bool> running{false};
    int server_fd = -1;

    std::vector<std::shared_ptr<Session>> sessions;
    std::mutex sessions_mutex_;

    /// All player sessions handled by the server share the same game controller
    /// (which includes the common game context shared by all players).
    std::shared_ptr<GameController> shared_controller_;
};
