#include "MessageRouter.hpp"

#include <nlohmann/json.hpp>

#include "GameContext.hpp"
#include "Logger.hpp"
#include "MoveParser.hpp"

using json = nlohmann::json;

MessageRouter::MessageRouter() : game_context_(std::make_unique<GameContext>()) {
    auto& logger = Logger::instance();
    logger.info("MessageRouter initialized with GameContext FSM");
}

std::string MessageRouter::route(const nlohmann::json& msg, const std::string& session_id) {
    auto& logger = Logger::instance();

    try {
        logger.debug("Routing message for session: " + session_id);

        // Command-based routing
        if (msg.contains("command")) {
            std::string command = msg["command"];
            
            if (command == "join_game") {
                std::string color = msg["color"];
                return handleJoinGame(session_id, color);
            }
            else if (command == "start_game") {
                return handleStartGame(session_id);
            }
            else if (command == "make_move") {
                std::string from = msg["from"];
                std::string to = msg["to"];
                return handleMakeMove(session_id, from, to);
            }
            else if (command == "end_game") {
                return handleEndGame(session_id);
            }
            else if (command == "get_status") {
                return handleGetStatus();
            }
            else if (command == "display_board") {
                return handleDisplayBoard();
            }
            else if (command == "play_game") {
                std::string content = msg["content"];
                std::string filename = msg["filename"];
                return handleUploadGame(content, filename);
            }
        }

        // Simple move notation (backward compatibility)
        if (msg.contains("move")) {
            if (!msg["move"].is_string()) {
                json error;
                error["error"] = "Invalid message format";
                error["expected"] = "'move' must be a string";
                return error.dump();
            }

            std::string move = msg["move"];
            return handleSimpleMove(move);
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

std::string MessageRouter::handleJoinGame(const std::string& session_id, const std::string& color) {
    auto& logger = Logger::instance();
    logger.info("Session " + session_id + " joining as " + color);
    
    json response = game_context_->handleJoinRequest(session_id, color);
    return response.dump();
}

std::string MessageRouter::handleStartGame(const std::string& session_id) {
    auto& logger = Logger::instance();
    logger.info("Session " + session_id + " starting game");
    
    json response = game_context_->handleStartRequest(session_id);
    return response.dump();
}

std::string MessageRouter::handleMakeMove(const std::string& session_id, 
                                         const std::string& from, 
                                         const std::string& to) {
    auto& logger = Logger::instance();
    logger.info("Session " + session_id + " move: " + from + "-" + to);
    
    json response = game_context_->handleMoveRequest(session_id, from, to);
    return response.dump();
}

std::string MessageRouter::handleEndGame(const std::string& session_id) {
    auto& logger = Logger::instance();
    logger.info("Session " + session_id + " ending game");
    
    json response = game_context_->handleEndRequest(session_id);
    return response.dump();
}

std::string MessageRouter::handleGetStatus() {
    auto& logger = Logger::instance();
    logger.debug("Getting game status");
    
    json response;
    response["type"] = "status";
    response["message"] = game_context_->getStatusMessage();
    return response.dump();
}

std::string MessageRouter::handleDisplayBoard() {
    auto& logger = Logger::instance();
    logger.debug("Displaying board");

    try {
        auto* controller = game_context_->getGameController();
        if (!controller) {
            json error;
            error["type"] = "error";
            error["error"] = "Game not initialized";
            return error.dump();
        }

        return controller->handleDisplayBoard();

    } catch (const std::exception& e) {
        logger.critical("Exception in handleDisplayBoard: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Internal server error";
        return error.dump();
    }
}

std::string MessageRouter::handleUploadGame(const std::string& content, const std::string& filename) {
    auto& logger = Logger::instance();
    logger.info("Processing game file: " + filename);

    try {
        MoveParser parser;
        auto moves = parser.parseGame(content); 

        logger.info("Found " + std::to_string(moves.size()) + " moves in " + filename);

        json responses = json::array();

        for (size_t i = 0; i < moves.size(); ++i) {
            const auto& move = moves[i];
            
            // Each move goes through GameContext FSM
            std::string move_response = handleMakeMove("file_" + filename, move.from, move.to);
            json move_json = json::parse(move_response);
            responses.push_back(move_json);

            if (move_json.contains("type") && move_json["type"] == "error") {
                logger.warning("Error at move " + std::to_string(i + 1) + ": " + 
                             move.from + "-" + move.to);
                break;
            }
        }

        json final_response;
        final_response["type"] = "game_response";
        final_response["filename"] = filename;
        final_response["total_moves"] = responses.size();
        final_response["moves"] = responses;

        logger.info("Game file processed: " + std::to_string(responses.size()) + " moves");
        return final_response.dump();

    } catch (const std::exception& e) {
        logger.critical("Exception in handleUploadGame: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Failed to process game file";
        return error.dump();
    }
}

std::string MessageRouter::handleSimpleMove(const std::string& move) {
    auto& logger = Logger::instance();

    try {
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

        const auto& [from, to] = *parsed;
        logger.debug("Parsed simple move: " + from + " -> " + to);

        // Route through GameContext FSM
        return handleMakeMove("simple_move_session", from, to);

    } catch (const std::exception& e) {
        logger.critical("Exception in handleSimpleMove: " + std::string(e.what()));

        json error;
        error["error"] = "Internal server error";
        return error.dump();
    }
}