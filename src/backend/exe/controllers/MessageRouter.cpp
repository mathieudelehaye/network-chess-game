#include "MessageRouter.hpp"

#include <nlohmann/json.hpp>

#include "GameController.hpp"
#include "Logger.hpp"
#include "MoveParser.hpp"

using json = nlohmann::json;

MessageRouter::MessageRouter() : gameController_(std::make_unique<GameController>()) {
    // Start a new game on construction
    gameController_->startNewGame();
}

std::string MessageRouter::route(const nlohmann::json& msg) {
    auto& logger = Logger::instance();

    try {
        // Route based on message type
        if (msg.contains("move")) {
            if (!msg["move"].is_string()) {
                json error;
                error["error"] = "Invalid message format";
                error["expected"] = "'move' must be a string";
                return error.dump();
            }

            std::string move = msg["move"];
            logger.debug("Routing move command: " + move);
            return handleMove(move);
        }

        // Handle display_board command
        if (msg.contains("command") && msg["command"] == "display_board") {
            logger.debug("Routing display_board command");
            return handleDisplayBoard();
        }

        // Unknown message type
        logger.warning("Unknown message type");
        json error;
        error["error"] = "Unknown command";
        return error.dump();

    } catch (const json::exception& e) {
        logger.error("JSON processing error: " + std::string(e.what()));

        json error;
        error["error"] = "Invalid JSON structure";
        return error.dump();

    } catch (const std::exception& e) {
        logger.critical("Unexpected error in route: " + std::string(e.what()));

        json error;
        error["error"] = "Internal server error";
        return error.dump();
    }
}

std::string MessageRouter::handleMove(const std::string& move) {
    auto& logger = Logger::instance();

    try {
        // Step 1: Parse and validate move syntax using MoveParser
        MoveParser parser;
        auto parsed = parser.parse(move);

        if (!parsed) {
            logger.warning("Invalid move syntax: " + move);

            json error;
            error["error"] = "Invalid move syntax";
            error["received"] = move;
            error["expected"] = "Format: <start>-<end>, e.g., e2-e4";
            return error.dump();
        }

        // Step 2: Use ParsedMove to validate and apply chess move
        const auto& [from, to] = *parsed;
        logger.debug("Parsed move: " + from + " -> " + to);

        return gameController_->handleMove(from, to);

    } catch (const std::exception& e) {
        logger.critical("Exception in handleMove: " + std::string(e.what()));

        json error;
        error["error"] = "Internal server error";
        return error.dump();
    }
}

std::string MessageRouter::handleDisplayBoard() {
    auto& logger = Logger::instance();

    try {
        std::string boardASCII = gameController_->getGame().getBoardASCII();

        json response;
        response["board"] = boardASCII;
        response["status"] = "ok";

        logger.debug("Sent board display");
        return response.dump();

    } catch (const std::exception& e) {
        logger.critical("Exception in handleDisplayBoard: " + std::string(e.what()));

        json error;
        error["error"] = "Internal server error";
        return error.dump();
    }
}
