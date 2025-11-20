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
    void setupGameContextBroadcast();

    NetworkMode network;
    std::jthread acceptThread;
    std::atomic<bool> running{false};
    int server_fd = -1;

    std::vector<std::shared_ptr<Session>> sessions;
    std::mutex sessions_mutex_;

    /// Each player session handled by the server has its own game controller,
    /// and all game controllers share the same game context(which includes the game state).
    std::shared_ptr<GameContext> shared_game_context_;
};
