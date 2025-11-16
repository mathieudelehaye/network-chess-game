#include "Session.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

#include "Logger.hpp"
#include "MessageRouter.hpp"

using json = nlohmann::json;

Session::Session(std::unique_ptr<ITransport> t) : transport(std::move(t)) {}

Session::~Session() {
    close();
}

void Session::start() {
    active = true;

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

void Session::handleMessage(const std::string& json_str) {
    if (!active)
        return;

    auto& logger = Logger::instance();
    logger.trace("Received: " + json_str);

    try {
        // Parse JSON and check its correct syntax
        json msg = json::parse(json_str);

        // Route to controller
        std::string response = router->route(msg);

        // Send response back
        if (!response.empty()) {
            send(response);
            logger.trace("Sent response: " + response);
        }

    } catch (const json::exception& e) {
        logger.error("Invalid JSON: " + std::string(e.what()));

        json error_response;
        error_response["error"] = "Invalid JSON format";
        send(error_response.dump());

    } catch (const std::exception& e) {
        logger.error("Internal error: " +
                     std::string(e.what()));  // server logs internal error detail

        json error_response;
        error_response["error"] =
            "Internal server error";  // then sends a generic error message to the client
        send(error_response.dump());
    }
}

// void Session::send(const nlohmann::json& msg) {
void Session::send(const std::string& msg) {
    if (!active)
        return;
    // transport->send(msg.dump() + "\n");
    transport->send(msg + "\n");
}

void Session::close() {
    if (!active.exchange(false))
        return;

    transport->close();  // shuts down socket
}