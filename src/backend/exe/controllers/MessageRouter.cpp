#include "MessageRouter.hpp"

#include <nlohmann/json.hpp>

#include "Logger.hpp"
#include "MoveParser.hpp"

using json = nlohmann::json;

std::string MessageRouter::route(const nlohmann::json& msg) {
    auto& logger = Logger::instance();

    // Route based on message type
    if (msg.contains("move")) {
        std::string move = msg["move"];
        logger.debug("Routing move command: " + move);
        return handleMove(move);
    }

    // Unknown message type
    logger.warning("Unknown message type");
    json error;
    error["error"] = "Unknown command";
    return error.dump();
}

std::string MessageRouter::handleMove(const std::string& move) {
    auto& logger = Logger::instance();

    // Parse and validate move syntax
    MoveParser parser;
    auto parsed = parser.parse(move);

    if (!parsed) {
        logger.warning("Invalid move syntax: " + move);

        json error;
        error["error"] = "Invalid move syntax";
        error["received"] = move;
        error["expected"] = "Format: <start>-<end>, e.g., a2-a4";
        return error.dump();
    }

    // Extract coordinates
    const auto& [from, to] = *parsed;
    logger.info("Parsed move: " + from + " -> " + to);

    // TODO: Check if move is legal (not just syntactically valid)
    // TODO: Apply move to game state

    // Build response with extracted coordinates
    json response;
    response["status"] = "ok";
    response["move"] = {{"from", from}, {"to", to}, {"original", move}};
    response["message"] = "Piece moves from " + from + " to " + to;

    return response.dump();
}