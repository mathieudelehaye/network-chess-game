#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

#include "Logger.hpp"
#include "NetworkMode.hpp"
#include "ParserFactory.hpp"
#include "Server.hpp"

using namespace std;

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS]\n"
              << "Options:\n"
              << "  --parser <type>    Parser type: 'simple' or 'pgn' (default: simple)\n"
              << "  --port <port>      Server port (default: 8080)\n"
              << "  --help             Show this help message\n";
}

int main(int argc, char* argv[]) {
    auto& logger = Logger::instance();

    // Default values
    NetworkMode network = NetworkMode::TCP;
    std::string ip_address = "127.0.0.1";
    int port = 2000;
    bool verbose = false;
    bool help = false;
    std::string socket_path = "/tmp/chess_server.sock";
    ParserType parser = ParserType::SIMPLE_NOTATION;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--ip" && i + 1 < argc) {
            ip_address = argv[++i];
        } else if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--socket" && i + 1 < argc) {
            socket_path = argv[++i];
        } else if (arg == "--parser" && i + 1 < argc) {
            std::string parser_arg = argv[++i];
            logger.debug("Parser changed to: " +  parser_arg); 
            
            if (parser_arg == "pgn") {
                parser = ParserType::PGN;
            } else if (parser_arg == "simple") {
                parser = ParserType::SIMPLE_NOTATION;
            } else {
                logger.error("Unknown parser type: " + parser_arg);
                return 1;
            }
        } else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else if (arg == "--help" || arg == "-h") {
            help = true;
        }
    }

    try {
        logger.info("Starting chess server...");
        logger.info("Parser type: " + (parser==ParserType::PGN) ? "PGN":"Simple");
        logger.info("Port: " + std::to_string(port));

        Server server(network, port, parser);

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