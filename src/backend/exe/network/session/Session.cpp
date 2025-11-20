#include "Session.hpp"

#include <iostream>
#include <string>

#include "GameController.hpp"
#include "Logger.hpp"

Session::Session(std::unique_ptr<ITransport> transport, std::shared_ptr<GameContext> game_context)
    : transport(std::move(transport)),
      controller(std::make_unique<GameController>(game_context)),
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
    if (active.exchange(true)) {
        return;  // Already started
    }

    auto& logger = Logger::instance();
    logger.info("Session started: " + session_id_);

    // Send handshake as part of session initialisation
    json handshake = {{"type", "session_created"}, {"session_id", session_id_}};
    send(handshake.dump());

    // Start receiving messages
    transport->start([this](const std::string& payload) { onReceive(payload); });
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