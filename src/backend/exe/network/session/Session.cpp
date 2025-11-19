#include "Session.hpp"

#include <iostream>

#include "GameController.hpp"
#include "Logger.hpp"

Session::Session(std::unique_ptr<ITransport> t)
    : router(std::make_unique<GameController>()),
      transport(std::move(t)),
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
    // Session layer just routes raw string.
    std::string response = router->routeMessage(message, session_id_);

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

    auto& logger = Logger::instance();
    logger.info("Session closed: " + session_id_);
}

std::string Session::generateSessionId() {
    static std::atomic<uint64_t> counter{0};
    return "session_" + std::to_string(++counter);
}