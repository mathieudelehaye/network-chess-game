#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>

#include "GameController.hpp"
#include "ITransport.hpp"

/**
 * @brief Represents a single connected client session.
 *
 * The Session owns an ITransport (Strategy), parses JSON, and routes commands
 * using GameController.
 *
 * TODO: check if this wrapper Session layer can be removed, since it only helps
 * receive split messages for now.
 */
class Session {
   public:
    explicit Session(std::unique_ptr<ITransport> transport);
    ~Session();

    void start();                       ///< Start receiving messages
    void send(const std::string& msg);  ///< Send message over transport
    void close();                       ///< Shutdown session

   private:
    void onReceive(
        const std::string& raw);  ///< Accumulate payloads until having received a complete message
    void handleMessage(const std::string& message);  ///< Route complete message

    static std::string generateSessionId();  ///< Generate unique session ID

    std::unique_ptr<GameController> router;
    std::unique_ptr<ITransport> transport;
    std::atomic<bool> active{
        false};          /// Useful to avoid passing messages in callback functions during shutdown.
    std::string buffer;  /// Buffer to accumulate message fragments
    std::string session_id_;  ///< Unique identifier for this session
};