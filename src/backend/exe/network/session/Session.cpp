#include "Session.hpp"

#include <iostream>

Session::Session(std::unique_ptr<ITransport> t) : transport(std::move(t)) {}

Session::~Session() {
    close();
}

void Session::start() {
    active = true;

    readerThread = std::jthread([this](std::stop_token st) { this->readerLoop(st); });

    transport->start([this](const std::string& msg) { handleMessage(msg); });
}

void Session::readerLoop(std::stop_token st) {
    while (!st.stop_requested() && active.load()) {
        // The transport layer actually triggers callbacks, but we keep this loop method only to
        // detect stop_token status.
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void Session::handleMessage(const std::string& raw) {
    if (!active)
        return;

    try {
        // nlohmann::json req = nlohmann::json::parse(raw);
        // nlohmann::json resp = router.route(req);
        // send(resp);
    } catch (const std::exception& e) {
        // send({{"error", e.what()}});
        send("error");
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

    readerThread.request_stop();  // jthread cancel
    transport->close();           // shuts down socket
}