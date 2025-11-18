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

        // Build JSON response from StrikeData
        json response;
        response["status"] = "ok";
        response["strike_number"] = strikeData->strike_number;
        response["piece"] = strikeData->piece;
        response["color"] = strikeData->color;
        response["case_src"] = strikeData->case_src;
        response["case_dest"] = strikeData->case_dest;

        // Add optional fields
        if (strikeData->is_capture) {
            response["is_capture"] = true;
            response["captured_piece"] = strikeData->captured_piece;
            response["captured_color"] = strikeData->captured_color;
        }

        if (strikeData->is_castling) {
            response["is_castling"] = true;
            response["castling_type"] = strikeData->castling_type;
        }

        if (strikeData->is_check) {
            response["is_check"] = true;
        }

        if (strikeData->is_checkmate) {
            response["is_checkmate"] = true;
            response["game_over"] = true;
            response["result"] = "checkmate";
        }

        if (strikeData->is_stalemate) {
            response["is_stalemate"] = true;
            response["game_over"] = true;
            response["result"] = "stalemate";
        }

        logger.info("Move applied successfully");
        return response.dump();

    } catch (const std::exception& e) {
        logger.critical("Exception in handleMove: " + std::string(e.what()));

        json error;
        error["error"] = "Internal server error";
        return error.dump();
    }
}