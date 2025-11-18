#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "ITransport.hpp"
#include "MessageRouter.hpp"

// Structure to track file upload progress
struct FileUploadState {
    std::string filename;
    int total_size = 0;
    int chunks_total = 0;
    int chunks_received = 0;
    std::string accumulated_data;  // Reassembled file content
};

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
                                                      /// before passing it to
                                                      /// the controller layer.
    void handleFileUpload(const nlohmann::json& msg);
    void processGameFile(const std::string& content, const std::string& filename);

    std::unique_ptr<MessageRouter> router;
    std::unique_ptr<ITransport> transport;
    std::atomic<bool> active{false};  /// Useful to avoid passing messages in callback functions during shutdown.
    std::string buffer;  /// Buffer to acumulate message fragments
    
    // Track ongoing file uploads
    std::unordered_map<std::string, FileUploadState> file_uploads_;
};
