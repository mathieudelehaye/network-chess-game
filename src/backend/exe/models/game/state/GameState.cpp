#include "GameState.hpp"

#include "GameContext.hpp"
#include "Logger.hpp"

json WaitingForPlayersState::handleJoinRequest(GameContext* context, const std::string& player_id,
                                               const std::string& color) {
    auto& logger = Logger::instance();

    if (color == "white") {
        if (context->hasWhitePlayer() && context->getWhitePlayer() != player_id) {
            return buildError("White player slot already taken");
        }
        context->setWhitePlayer(player_id);
        logger.info("Player " + player_id + " joined as White");
    } else if (color == "black") {
        if (context->hasBlackPlayer() && context->getBlackPlayer() != player_id) {
            return buildError("Black player slot already taken");
        }
        context->setBlackPlayer(player_id);
        logger.info("Player " + player_id + " joined as Black");
    } else {
        return buildError("Invalid color");
    }

    // Check if it's single-player (same session ID for both colors)
    bool is_single_player = (context->getWhitePlayer() == context->getBlackPlayer());

    if (context->bothPlayersJoined()) {
        context->transitionTo(std::make_unique<ReadyToStartState>());

        if (is_single_player) {
            logger.info("Single-player mode detected");
        }
    }

    return json{{"type", "join_success"},
                {"color", color},
                {"status", context->getStatusMessage()},
                {"single_player", is_single_player}};
}

json ReadyToStartState::handleStartRequest(GameContext* context, const std::string& player_id) {
    auto& logger = Logger::instance();

    if (player_id != context->getWhitePlayer() && player_id != context->getBlackPlayer()) {
        return buildError("Only joined players can start the game");
    }

    context->transitionTo(std::make_unique<InProgressState>());

    logger.info("Game started");

    return json{{"type", "game_started"}, {"status", context->getStatusMessage()}};
}

json ReadyToStartState::handleEndRequest(GameContext* context, const std::string& /*player_id*/) {
    // Reset and go back to waiting
    context->transitionTo(std::make_unique<WaitingForPlayersState>());

    return json{{"type", "game_reset"}, {"status", "Waiting for new players"}};
}

json InProgressState::handleMoveRequest(GameContext* context, const std::string& /*player_id*/,
                                        const std::string& from, const std::string& to) {
    auto* game = context->getChessGame();
    if (!game) {
        return buildError("Game not initialized");
    }

    // Apply move directly to model (no controller layer needed)
    auto strike_data = game->applyMove(from, to);

    if (!strike_data) {
        return buildError("Invalid move");
    }

    // Build response
    json response;
    response["type"] = "move_result";
    response["success"] = true;
    response["strike"] = {
        {"from", strike_data->case_src},         {"to", strike_data->case_dest},
        {"piece", strike_data->piece},           {"capture", strike_data->is_capture},
        {"check", strike_data->is_check},        {"checkmate", strike_data->is_checkmate},
        {"stalemate", strike_data->is_stalemate}};

    // Check if game ended
    if (strike_data->is_checkmate || strike_data->is_stalemate) {
        context->transitionTo(std::make_unique<GameOverState>());

        auto& logger = Logger::instance();
        if (strike_data->is_checkmate) {
            logger.info("Game over - Checkmate!");
        } else {
            logger.info("Game over - Stalemate");
        }
    }

    return response;
}

json InProgressState::handleEndRequest(GameContext* context, const std::string& player_id) {
    auto& logger = Logger::instance();
    logger.info("Game ended by player: " + player_id);

    context->transitionTo(std::make_unique<GameOverState>());

    return json{{"type", "game_ended"}, {"status", "Game ended by player"}};
}

json InProgressState::handleDisplayBoard(GameContext* context) {
    auto* game = context->getChessGame();
    if (!game) {
        return buildError("Game not initialized");
    }

    try {
        std::string boardASCII = game->getBoardASCII();

        json response;
        response["type"] = "board_display";
        response["status"] = "ok";
        response["data"]["board"] = boardASCII;

        return response;

    } catch (const std::exception& e) {
        auto& logger = Logger::instance();
        logger.error("Failed to display board: " + std::string(e.what()));

        return buildError("Failed to display board");
    }
}

json GameOverState::handleEndRequest(GameContext* context, const std::string& player_id) {
    auto& logger = Logger::instance();
    logger.info("Game reset requested by player: " + player_id);

    // Reset to waiting for new game
    context->transitionTo(std::make_unique<WaitingForPlayersState>());

    return json{{"type", "game_reset"}, {"status", "Ready for new game"}};
}