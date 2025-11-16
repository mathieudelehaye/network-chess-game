#include "Session.hpp"

#include <iostream>

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

    try {
        // Now we can safely parse - we have a complete message
        // nlohmann::json req = nlohmann::json::parse(json_str);

        // Route and respond
        // nlohmann::json resp = router.route(req);
        // send(resp.dump());

    } catch (const std::exception& e) {
        std::cerr << "Message handling error: " << e.what() << "\n";
        // send(nlohmann::json{{"error", e.what()}}.dump());
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