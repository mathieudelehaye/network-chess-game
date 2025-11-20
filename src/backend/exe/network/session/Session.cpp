#include "Session.hpp"

#include <iostream>
#include <string>

#include "GameController.hpp"
#include "Logger.hpp"

Session::Session(std::unique_ptr<ITransport> transport, std::shared_ptr<GameController> controller)
    : transport(std::move(transport)),
      controller(std::move(controller)),
      session_id_(generateSessionId()) {
    auto& logger = Logger::instance();
    logger.info("Session created: " + session_id_);
}

Session::~Session() {
    close();
}

void Session::start() {
    // Required since nothing prevents start() from being called twice for the same instance.
    if (active.exchange(true))
        return;

    auto& logger = Logger::instance();
    logger.info("Session started: " + session_id_);

    // Set close callback BEFORE starting transport
    // Use weak_ptr to avoid circular reference
    std::weak_ptr<Session> weak_self = shared_from_this();
    transport->setCloseCallback([weak_self]() {
        if (auto self = weak_self.lock()) {
            auto& logger = Logger::instance();
            logger.info("Transport closed unexpectedly for session: " + self->session_id_);
            self->close();  // Trigger session cleanup
        }
    });

    // Start receiving messages
    transport->start([this](const std::string& payload) { onReceive(payload); });

    // Send handshake as part of session initialisation
    json handshake = {{"type", "session_created"}, {"session_id", session_id_}};
    send(handshake.dump());
}

void Session::onReceive(const std::string& raw) {
    // Required to prevent callback function from being called during shutdown.
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
    // Required to prevent sending messages through an inactive session.
    if (!active)
        return;
    transport->send(msg + "\n");
}

void Session::close() {
    // Required to be consistent with the similar check in start().
    if (!active.exchange(false))
        return;

    transport->close();

    // Notify GameContext that this player disconnected
    if (controller) {
        controller->routeDisconnect(session_id_);
    }

    auto& logger = Logger::instance();
    logger.info("Session closed: " + session_id_);
}

std::string Session::generateSessionId() {
    static std::atomic<uint64_t> counter{0};
    return "session_" + std::to_string(++counter);
}