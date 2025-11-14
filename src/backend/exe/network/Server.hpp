#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "NetworkMode.hpp"
#include "Session.hpp"
#include "TransportFactory.hpp"

class Server {
   public:
    Server(NetworkMode mode, const std::string& ip, int port);

    void start();
    void stop();

   private:
    void acceptLoop(std::stop_token st);

   private:
    int server_fd = -1;

    void connectTCP(const std::string& ip, int port);
    void connectIPC();

    NetworkMode network;

    std::jthread acceptThread;
    std::atomic<bool> running{false};

    std::vector<std::shared_ptr<Session>> sessions;
};
