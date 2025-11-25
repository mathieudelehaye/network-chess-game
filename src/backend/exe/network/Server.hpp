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

/**
 * @class Server
 * @brief Chess game server managing client connections and sessions.
 *
 * Handles TCP/IPC socket binding, accepts connections, manages session lifecycle,
 * and provides broadcast/unicast messaging. All sessions share a common GameController.
 */
class Server {
   public:
    /**
     * @brief Construct server with network mode, port, and parser type.
     * @param mode Network mode (TCP or IPC)
     * @param port Server port for TCP mode
     * @param parser Parser type for game notation
     */
    Server(NetworkMode mode, int port, ParserType parser);

    /**
     * @brief Destructor stops server and cleans up resources.
     */
    ~Server() { stop(); }

    /**
     * @brief Start TCP server on specified IP address.
     * @param ip IP address to bind (e.g., "0.0.0.0", "127.0.0.1")
     */
    void start(const std::string& ip);

    /**
     * @brief Start Unix domain socket server.
     * @param socket_path Path to Unix socket file
     */
    void start_unix(const std::string& socket_path);

    /**
     * @brief Start accept and cleanup background threads.
     */
    void start_threads();

    /**
     * @brief Stop server and terminate all sessions.
     */
    void stop();

   private:
    /**
     * @brief Setup send callbacks for controller to route messages.
     */
    void setupSendCallbacks();

    /**
     * @brief Accept loop - handles incoming connections.
     * @param st Stop token for thread termination
     */
    void acceptLoop(std::stop_token st);

    /**
     * @brief Cleanup loop - removes closed sessions periodically.
     * @param st Stop token for thread termination
     */
    void cleanupLoop(std::stop_token st);

    /**
     * @brief Handle session closure event.
     * @param session_id ID of closed session
     */
    void handleSessionClosed(const std::string& session_id);

    /**
     * @brief Remove closed sessions from active sessions map.
     */
    void cleanupClosedSessions();

    /**
     * @brief Connect TCP socket.
     * @param ip IP address to bind
     * @param port Port number
     */
    void connectTCP(const std::string& ip, int port);

    /**
     * @brief Connect Unix domain socket.
     * @param socket_path Path to socket file
     */
    void connectIPC(const std::string& socket_path);

    /**
     * @brief Broadcast message to all connected sessions.
     * @param message Message to broadcast
     */
    void broadcastToAll(const std::string& message);

    /**
     * @brief Broadcast message to all sessions except one.
     * @param exclude_session_id Session ID to exclude from broadcast
     * @param message Message to broadcast
     */
    void broadcastToOthers(const std::string& exclude_session_id, const std::string& message);

    /**
     * @brief Send message to specific session.
     * @param session_id Target session ID
     * @param message Message to send
     */
    void unicastTo(const std::string& session_id, const std::string& message);

    NetworkMode network;            ///< Network mode (TCP/IPC)
    int port;                       ///< Server port (TCP mode)
    int server_fd = -1;             ///< Server socket file descriptor
    std::string unix_socket_path_;  ///< Unix socket path (IPC mode)

    std::atomic<bool> running{false};  ///< Server running flag

    std::map<std::string, std::shared_ptr<Session>> sessions;  ///< Active sessions map
    std::mutex sessions_mutex_;                                ///< Mutex for sessions access

    std::vector<std::string> sessions_to_cleanup_;  ///< Closed sessions queue
    std::mutex cleanup_mutex_;                      ///< Mutex for cleanup queue

    std::jthread acceptThread;   ///< Accept loop thread
    std::jthread cleanupThread;  ///< Cleanup loop thread

    /// All player sessions share the same GameController (common GameContext).
    std::shared_ptr<GameController> shared_controller_;
};