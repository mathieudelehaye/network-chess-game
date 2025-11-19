#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

#include "network/NetworkMode.hpp"
#include "network/Server.hpp"
#include "utils/Logger.hpp"

using namespace std;

int main() {
    auto& logger = Logger::instance();

    try {
        logger.info("Starting chess server...");

        Server server(NetworkMode::TCP, "127.0.0.1", 2000);

        server.start();

        logger.info("Server running on 127.0.0.1:2000");
        std::cout << "Press Enter to stop..." << std::endl;

        std::cin.get();

        logger.info("Stopping server...");
        server.stop();
    } catch (const std::runtime_error& e) {
        logger.critical("Server initialization failed: " + std::string(e.what()));
        return 1;
    } catch (const std::exception& e) {
        logger.critical("Unexpected error: " + std::string(e.what()));
        return 2;
    }

    return 0;
}