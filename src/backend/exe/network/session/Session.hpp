#pragma once

#include <atomic>
#include <memory>
// #include <nlohmann/json.hpp>
#include <string>
#include <thread>

#include "ITransport.hpp"
#include "MessageRouter.hpp"

/**
 * @brief Represents a single connected client session.
 *
 * Uses std::jthread for cooperative cancellation.
 * The Session owns an ITransport (Strategy), parses JSON,
 * and routes commands using MessageRouter.
 */
class Session {
   public:
    explicit Session(std::unique_ptr<ITransport> transport);
    ~Session();

    void start();  ///< Start receiving messages
    // void send(const nlohmann::json& msg);  ///< Send JSON over transport
    void send(const std::string& msg);  ///< Send JSON over transport
    void close();                       ///< Shutdown transport + thread

   private:
    void onReceive(
        const std::string& raw);  /// Wait until complete application message is received.
    void handleMessage(const std::string& json_str);  /// Check for application message syntax
                                                      /// before passing it to the controller layer.

    std::string buffer;  /// Buffer to acumulate message fragments
    std::atomic<bool> active{
        false};  /// Useful to avoid passing messages in callback functions during shutdown.
    std::unique_ptr<ITransport> transport;
    std::unique_ptr<MessageRouter> router;
};
