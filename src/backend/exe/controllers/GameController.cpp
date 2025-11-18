#include "GameController.hpp"

#include <nlohmann/json.hpp>

#include "Logger.hpp"

using json = nlohmann::json;

GameController::GameController() : game_(std::make_unique<ChessGame>()) {
    Logger::instance().trace("GameController created");
}

void GameController::startNewGame() {
    game_->reset();
    Logger::instance().info("New game started");
}

std::string GameController::handleMove(const std::string& from, const std::string& to) {
    auto& logger = Logger::instance();

    try {
        // Apply move and get strike data
        auto strikeData = game_->applyMove(from, to);

        if (!strikeData) {
            logger.warning("Illegal move attempted: " + from + "-" + to);

            json error;
            error["error"] = "Illegal move";
            error["move"] = from + "-" + to;
            return error.dump();
        }

        logger.debug("Building JSON response");
        
        // Build nested JSON response with explicit type
        json response;
        response["type"] = "move_response";
        response["status"] = "ok";

        // Data in specified order (though JSON doesn't guarantee order)
        json data;
        data["piece"] = strikeData->piece;
        data["color"] = strikeData->color;
        data["strike_number"] = strikeData->strike_number;
        data["case_src"] = strikeData->case_src;
        data["case_dest"] = strikeData->case_dest;

        // Add optional fields
        if (strikeData->is_capture) {
            data["is_capture"] = true;
            data["captured_piece"] = strikeData->captured_piece;
            data["captured_color"] = strikeData->captured_color;
        }

        if (strikeData->is_castling) {
            data["is_castling"] = true;
            data["castling_type"] = strikeData->castling_type;
        }

        if (strikeData->is_check) {
            data["is_check"] = true;
        }

        if (strikeData->is_checkmate) {
            data["is_checkmate"] = true;
            data["game_over"] = true;
            data["result"] = "checkmate";
        }

        if (strikeData->is_stalemate) {
            data["is_stalemate"] = true;
            data["game_over"] = true;
            data["result"] = "stalemate";
        }

        response["data"] = data;
        
        logger.info("Move applied successfully");
        return response.dump();

    } catch (const std::exception& e) {
        logger.critical("Exception in handleMove: " + std::string(e.what()));

        json error;
        error["error"] = "Internal server error";
        return error.dump();
    }
}

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