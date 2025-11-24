#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "GameContext.hpp"
#include "NetworkMode.hpp"
#include "ParserFactory.hpp"
#include "Session.hpp"
#include "TransportFactory.hpp"

class Server {
public:
    Server(NetworkMode mode, int port, ParserType parser);
    ~Server() { stop(); }

    void start(const std::string& ip);
    void start_unix(const std::string& socket_path);
    void start_threads();
    void stop();

private:
    void setupSendCallbacks();
    void acceptLoop(std::stop_token st);
    void cleanupLoop(std::stop_token st);
    void handleSessionClosed(const std::string& session_id);
    void cleanupClosedSessions();

    void connectTCP(const std::string& ip, int port);
    void connectIPC(const std::string& socket_path);
    
    void broadcastToAll(const std::string& message);
    void broadcastToOthers(const std::string& exclude_session_id, const std::string& message);
    void unicastTo(const std::string& session_id, const std::string& message);

    NetworkMode network;
    int port;
    int server_fd = -1;
    std::string unix_socket_path_;
    
    std::atomic<bool> running{false};
    
    std::map<std::string, std::shared_ptr<Session>> sessions;
    std::mutex sessions_mutex_;
    
    std::vector<std::string> sessions_to_cleanup_;
    std::mutex cleanup_mutex_;
    
    std::jthread acceptThread;
    std::jthread cleanupThread;

    /// All player sessions handled by the server share the same game controller
    /// (which includes the common game context shared by all players).
    std::shared_ptr<GameController> shared_controller_;
};