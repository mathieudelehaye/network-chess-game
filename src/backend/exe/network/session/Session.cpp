#include "Session.hpp"

#include <iostream>

#include "Logger.hpp"
#include "MessageRouter.hpp"

using json = nlohmann::json;

Session::Session(std::unique_ptr<ITransport> t)
    : router(std::make_unique<MessageRouter>()),
      transport(std::move(t)),   
      session_id_(generateSessionId()),
      logger(Logger::instance()) {
    
    logger.info("Session created: " + session_id_);
}

Session::~Session() {
    close();
    logger.info("Session destroyed: " + session_id_);
}

void Session::start() {
    active = true;
    transport->start([this](const std::string& payload) { onReceive(payload); });
    logger.info("Session started: " + session_id_);
}

void Session::onReceive(const std::string& raw) {
    if (!active)
        return;

    // Accumulate data into buffer
    buffer += raw;

    // Process all complete messages (delimited by '\n')
    while (buffer.find('\n') != std::string::npos) {
        // Extract one complete message
        size_t pos = buffer.find('\n');
        std::string message = buffer.substr(0, pos);
        buffer.erase(0, pos + 1);

        // Parse and handle this complete message
        handleMessage(message);
    }
}

void Session::handleMessage(const std::string& message) {
    try {
        json msg = json::parse(message);
        
        // Check if this is a file upload chunk
        if (msg.contains("type") && msg["type"] == "file_upload") {
            handleFileUpload(msg);
            return;
        }
        
        // Route with session ID
        std::string response = router->route(msg, session_id_);
        
        if (!response.empty()) {
            transport->send(response);
        }
    } catch (const json::exception& e) {
        logger.error("JSON error: " + std::string(e.what()));
    }
}

void Session::handleFileUpload(const nlohmann::json& msg) {
    try {
        // Extract metadata
        auto metadata = msg["metadata"];
        std::string filename = metadata["filename"];
        int total_size = metadata["total_size"];
        int chunks_total = metadata["chunks_total"];
        int chunk_current = metadata["chunk_current"];
        std::string chunk_data = msg["data"];

        // Get or create upload state for this file
        auto& upload = file_uploads_[filename];

        // Initialize on first chunk
        if (chunk_current == 1) {
            upload.filename = filename;
            upload.total_size = total_size;
            upload.chunks_total = chunks_total;
            upload.chunks_received = 0;
            upload.accumulated_data.clear();
            upload.accumulated_data.reserve(total_size);  // Pre-allocate
            
            logger.info("Starting file upload: " + filename + 
                       " (" + std::to_string(total_size) + " bytes)");
        }

        // Append chunk data
        upload.accumulated_data += chunk_data;
        upload.chunks_received = chunk_current;

        // Calculate and log progress
        int percent = (chunk_current * 100) / chunks_total;
        logger.info("Downloading " + filename + ": " + 
                   std::to_string(percent) + "% (" + 
                   std::to_string(chunk_current) + "/" + 
                   std::to_string(chunks_total) + ")");

        // Send progress acknowledgment
        json ack;
        ack["type"] = "upload_progress";
        ack["filename"] = filename;
        ack["chunk_received"] = chunk_current;
        ack["chunks_total"] = chunks_total;
        ack["percent"] = percent;
        send(ack.dump());

        // Check if upload complete
        if (chunk_current >= chunks_total) {
            logger.info("File upload complete: " + filename + 
                       " (" + std::to_string(upload.accumulated_data.size()) + 
                       " bytes received)");

            // Process the complete file
            processGameFile(upload.accumulated_data, filename);

            // Clean up
            file_uploads_.erase(filename);
        }

    } catch (const json::exception& e) {
        logger.error("Error processing file upload chunk: " + std::string(e.what()));

        json error_response;
        error_response["type"] = "error";
        error_response["error"] = "Invalid upload chunk format";
        send(error_response.dump());
    }
}

void Session::processGameFile(const std::string& content, const std::string& filename) {
    json game_msg;
    game_msg["command"] = "play_game";
    game_msg["content"] = content;
    game_msg["filename"] = filename;
    
    std::string response = router->route(game_msg, session_id_);
    
    if (!response.empty()) {
        transport->send(response);
    }
}

void Session::send(const std::string& msg) {
    if (!active)
        return;
    transport->send(msg + "\n");
}

void Session::close() {
    if (!active.exchange(false))
        return;

    transport->close();
    logger.info("Session closed: " + session_id_);
}

std::string Session::generateSessionId() {
    static std::atomic<uint64_t> counter{0};
    return "session_" + std::to_string(++counter);
}