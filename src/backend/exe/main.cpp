#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <iostream>

#include "Logger.hpp"
#include "NetworkMode.hpp"
#include "ParserFactory.hpp"
#include "Server.hpp"

using namespace std;

void printUsage(const string& program_name) {
    cout << "Usage: " << program_name << " [OPTIONS]\n"
              << "Options:\n"
              << "  -h                  Show this help message\n"
              << "  -i <ip address>     Server ip address (default: 127.0.0.1)\n"
              << "  -p <port>           Server port (default: 2000)\n"
              << "  -v                  Show debug level logging\n"
              << "  --parser <type>     Parser type: 'simple' or 'pgn' (default: simple)\n"
              << "  --local             Use local IPC network (instead of TCP)\n"
              << "  --socket <socket>   Socket path (only for IPC) (default: `/tmp/chess_server.sock`)\n";
}

int main(int argc, char* argv[]) {
    auto& logger = Logger::instance();

    // Default values
    NetworkMode network = NetworkMode::TCP;
    string ip_address = "127.0.0.1";
    int port = 2000;
    string socket_path = "/tmp/chess_server.sock";
    ParserType parser = ParserType::SIMPLE_NOTATION;

    // Parse command line arguments
    const string program_name = argv[0];

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            printUsage(program_name);
            return 0;
        }else if ((arg == "--ip" || arg == "-i") && i + 1 < argc) {
            ip_address = argv[++i];
        } else if ((arg == "--port" || arg == "-p") && i + 1 < argc) {
            port = stoi(argv[++i]);
        } else if (arg == "--local") {
            network = NetworkMode::IPC;
            logger.info("Using IPC network protocol");
        } else if (arg == "--socket" && i + 1 < argc) {
            socket_path = argv[++i];
        } else if (arg == "--parser" && i + 1 < argc) {
            string parser_arg = argv[++i];
            if (parser_arg == "pgn") {
                parser = ParserType::PGN;
                logger.info("Parser changed to: " +  parser_arg); 
            } 
        } else if (arg == "--verbose" || arg == "-v") {
            logger.setLogLevel(spdlog::level::debug);
            logger.info("Log level set to Debug (instead of Info)"); 
        }
    }

    try {
        logger.info("Starting chess server...");
        logger.info("Parser type: " + ((parser == ParserType::PGN) ? string("PGN") : string("Simple")));
        logger.info("Port: " + to_string(port));

        Server server(network, port, parser);

        if (network == NetworkMode::IPC) {
            server.start_unix(socket_path);
        } else {
            server.start(ip_address);
        }

        logger.info("Server running on address: " + ((network == NetworkMode::IPC) ? socket_path : ip_address));
        cout << "Press Enter to stop..." << endl;

        cin.get();

        logger.info("Stopping server...");
        server.stop();
    } catch (const runtime_error& e) {
        logger.critical("Server initialisation failed: " + string(e.what()));
        return 1;
    } catch (const exception& e) {
        logger.critical("Unexpected error: " + string(e.what()));
        return 2;
    }

    return 0;
}