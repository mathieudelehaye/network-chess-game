#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

#include "network/NetworkMode.hpp"
#include "network/Server.hpp"
#include "utils/Logger.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    auto& logger = Logger::instance();

    NetworkMode network = NetworkMode::TCP;
    std::string ip_address = "127.0.0.1";
    int port = 2000;
    bool verbose = false;
    bool help = false;
    std::string socket_path = "/tmp/chess_server.sock";

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            ip_address = argv[++i];
        } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) {
            verbose = true;
            logger.info("Verbose (Debug) mode activated");
        } else if (strcmp(argv[i], "-h") == 0 && i + 1 < argc) {
            help = true;
        } else if (strcmp(argv[i], "-l") == 0) {
            network = NetworkMode::IPC;
            logger.info("Using Unix domain socket for IPC");
        } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
            socket_path = argv[++i];
        } 
    }

    try {
        logger.info("Starting chess server...");

        Server server(network, port);

        if (network == NetworkMode::IPC) {
            server.start_unix(socket_path);
        } else {
            server.start(ip_address);
        }

        logger.info("Server running on address: " + (network == NetworkMode::IPC) ? socket_path : ip_address);
        std::cout << "Press Enter to stop..." << std::endl;

        std::cin.get();

        logger.info("Stopping server...");
        server.stop();
    } catch (const std::runtime_error& e) {
        logger.critical("Server initialisation failed: " + std::string(e.what()));
        return 1;
    } catch (const std::exception& e) {
        logger.critical("Unexpected error: " + std::string(e.what()));
        return 2;
    }

    return 0;
}