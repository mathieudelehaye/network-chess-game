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

    // Broadcast method
    void broadcastToOthers(const std::string& exclude_session_id, const std::string& message);

private:
    void acceptLoop(std::stop_token st);
    void sendSessionIdToCLient(const Session& session);
    void connectTCP(const std::string& ip, int port);
    void connectIPC();

    int server_fd = -1;

    NetworkMode network;

    std::jthread acceptThread;
    std::atomic<bool> running{false};

    std::vector<std::shared_ptr<Session>> sessions;
    std::mutex sessions_mutex_;

    std::shared_ptr<GameContext> shared_game_context_;
};
