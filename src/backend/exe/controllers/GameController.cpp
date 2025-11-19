#include "GameController.hpp"

#include <nlohmann/json.hpp>
#include "Logger.hpp"

using json = nlohmann::json;

GameController::GameController():
    game_(std::make_unique<ChessGame>)
{}

// TODO: move function elsewhere
std::string GameController::handleDisplayBoard() {
    auto& logger = Logger::instance();

    try {
        // Get board from game model
        std::string boardASCII = game_->getBoardASCII();
        
        // Build JSON response
        json response;
        response["type"] = "board_display";
        response["status"] = "ok";
        response["data"]["board"] = boardASCII;
        
        logger.debug("Board display prepared");
        return response.dump();

    } catch (const std::exception& e) {
        logger.critical("Exception in handleDisplayBoard: " + std::string(e.what()));

        json response;
        response["type"] = "error";
        response["error"] = "Failed to display board";
        return response.dump();
    }
}