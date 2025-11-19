#include "Session.hpp"

#include <iostream>
#include <string>
#include "GameController.hpp"
#include "Logger.hpp"

Session::Session(
    std::unique_ptr<ITransport> transport, 
    BroadcastCallback broadcast_fn,
    std::shared_ptr<GameContext> game_context) : 
    transport(std::move(transport)),
    controller(std::make_unique<GameController>(game_context)),
    broadcast_fn_(std::move(broadcast_fn)),
    session_id_(generateSessionId()) {

    auto& logger = Logger::instance();
    logger.info("Session created: " + session_id_);
}

Session::~Session() {
    close();

    auto& logger = Logger::instance();
    logger.info("Session destroyed: " + session_id_);
}

void Session::start() {
    active = true;
    transport->start([this](const std::string& payload) { onReceive(payload); });
    auto& logger = Logger::instance();
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
    auto& logger = Logger::instance();
    logger.debug("Received: " + message);

    // Route message to game controller
    auto response = controller->routeMessage(message, session_id_);

    // Send response to requesting client
    send(response);
    logger.debug("Sent response: " + response); 

    // Check for broadcast message
    auto broadcast = controller->getAndClearPendingBroadcast();
    if (broadcast.has_value()) {
        std::string broadcast_str = broadcast.value().dump();
        std::string msg_type = broadcast.value().value("type", "");
        
        logger.debug("Preparing broadcast type: " + msg_type);
        
        // game_ready should go to ALL clients (including sender)
        // player_joined should go to OTHER clients (excluding sender)
        if (msg_type == "game_ready") {
            // Broadcast to ALL including self
            // We need to send to this session too!
            send(broadcast_str);
            // And broadcast to others
            broadcast_fn_(session_id_, broadcast_str);
        } else {
            // Broadcast to others only
            broadcast_fn_(session_id_, broadcast_str);
        }
        
        logger.debug("Broadcast sent");   
    }
}

void Session::send(const std::string& msg) const {
    if (!active)
        return;
    transport->send(msg + "\n");
}

void Session::close() {
    if (!active.exchange(false))
        return;

    transport->close();

    auto& logger = Logger::instance();
    logger.info("Session closed: " + session_id_);
}

std::string Session::generateSessionId() {
    static std::atomic<uint64_t> counter{0};
    return "session_" + std::to_string(++counter);
}