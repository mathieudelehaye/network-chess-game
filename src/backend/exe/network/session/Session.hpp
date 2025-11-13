#pragma once
#include <atomic>
#include <memory>
// #include <nlohmann/json.hpp>
#include <string>
#include <thread>

#include "ITransport.hpp"

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
    void readerLoop(std::stop_token st);  ///< Thread callback
    void handleMessage(const std::string& raw);

   private:
    std::unique_ptr<ITransport> transport;

    std::jthread readerThread;  ///< Background receive loop
    std::atomic<bool> active{false};

    // MessageRouter router;                  ///< Dispatch JSON → controllers
};
