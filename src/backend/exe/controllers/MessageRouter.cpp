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
        
        // Handle play_game command
        if (msg.contains("command") && msg["command"] == "play_game") {
            logger.debug("Routing play_game command");
            const std::string& content = msg["content"];
            const std::string& filename = msg["filename"];
            return handlePlayGame(content, filename);
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
    logger.debug("Delegating to GameController::handleDisplayBoard");

    try {
        // Delegate to GameController (which builds JSON)
        return gameController_->handleDisplayBoard();

    } catch (const std::exception& e) {
        logger.critical("Exception in handleDisplayBoard: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Internal server error";
        return error.dump();
    }
}

std::string MessageRouter::handlePlayGame(const std::string& content, const std::string& filename) {
    auto& logger = Logger::instance();
    logger.info("Processing game file: " + filename);

    try {
        // Parse entire game at once with ANTLR
        MoveParser parser;
        auto moves = parser.parseGame(content); 

        logger.info("Found " + std::to_string(moves.size()) + " moves in " + filename);

        // Apply each move
        json responses = json::array();

        for (size_t i = 0; i < moves.size(); ++i) {
            const auto& move = moves[i];
            
            // Apply move to game controller
            std::string move_response = gameController_->handleMove(move.from, move.to);
            json move_json = json::parse(move_response);
            responses.push_back(move_json);

            // Stop on error
            if (move_json["type"] == "error") {
                logger.warning("Error at move " + std::to_string(i + 1) + ": " + 
                             move.from + "-" + move.to);
                break;
            }
        }

        // Build final response
        json final_response;
        final_response["type"] = "game_response";
        final_response["filename"] = filename;
        final_response["total_moves"] = responses.size();
        final_response["moves"] = responses;

        logger.info("Game file processed: " + std::to_string(responses.size()) + " moves");
        return final_response.dump();

    } catch (const std::exception& e) {
        logger.critical("Exception in handlePlayGame: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Failed to process game file";
        return error.dump();
    }
}