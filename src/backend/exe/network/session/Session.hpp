#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>

#include "GameController.hpp"
#include "ITransport.hpp"

using CloseCallback = std::function<void(const std::string& session_id)>;

/**
 * @brief Represents a single connected client session.
 *
 * The Session owns an ITransport (Strategy), parses JSON, and routes commands
 * using GameController.
 */
class Session : public std::enable_shared_from_this<Session> {
   public:
    explicit Session(std::unique_ptr<ITransport> transport,
                     std::shared_ptr<GameController> controller);
    ~Session();

    void start();                                             ///< Start receiving messages
    void send(const std::string& msg) const;                  ///< Send message over transport
    void close();                                             ///< Shutdown session
    std::string getSessionId() const { return session_id_; }  ///< Getter for the session id
    bool isActive() const { return active.load(); }

    void setCloseCallback(CloseCallback callback);

   private:
    void onReceive(
        const std::string& raw);  ///< Accumulate payloads until having received a complete message
    void handleMessage(const std::string& message);  ///< Route complete message
    static std::string generateSessionId();          ///< Generate unique session ID

    std::unique_ptr<ITransport> transport;
    std::shared_ptr<GameController> controller;
    CloseCallback on_close_callback;
    std::string session_id_;  ///< Unique identifier for this session
    std::atomic<bool> active{
        false};          /// Useful to avoid passing messages in callback functions during shutdown.
    std::string buffer;  /// Buffer to accumulate message fragments
};