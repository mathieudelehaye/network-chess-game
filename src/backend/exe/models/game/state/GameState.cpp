#include "GameState.hpp"

#include "GameContext.hpp"
#include "Logger.hpp"

json WaitingForPlayersState::handleJoinRequest(GameContext* context, const std::string& player_id,
                                               const std::string& color) {
    auto& logger = Logger::instance();

    // Validate and assign player color
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

    // Build response for the joining player
    json join_response = {
        {"type", "join_success"},
        {"session_id", player_id},
        {"color", color},
        {"status", context->getStatusMessage()},
        {"single_player", is_single_player}
    };

    // Check if both players have joined
    if (context->bothPlayersJoined()) {
        // Transition to ReadyToStartState
        context->transitionTo(std::make_unique<ReadyToStartState>());

        if (is_single_player) {
            logger.info("Single-player mode detected");
        } else {
            logger.info("Both players joined! Ready to start.");
        }

        // Broadcast game_ready to ALL clients (both players can now start)
        json ready_broadcast = {
            {"type", "game_ready"},
            {"status", "Both players joined. You can now start the game!"},
            {"white_player", context->getWhitePlayer()},
            {"black_player", context->getBlackPlayer()},
            {"single_player", is_single_player}
        };
        context->setPendingBroadcast(ready_broadcast);

    } else {
        // Only one player joined so far
        // Broadcast player_joined to OTHER clients
        json player_joined_broadcast = {
            {"type", "player_joined"},
            {"color", color},
            {"status", context->getStatusMessage()}
        };
        context->setPendingBroadcast(player_joined_broadcast);
    }

    return join_response;
}

json ReadyToStartState::handleStartRequest(GameContext* context, const std::string& player_id) {
    auto& logger = Logger::instance();
    logger.info("Session " + player_id + " starting game");

    // Transition to InProgressState
    context->transitionTo(std::make_unique<InProgressState>());

    // Initialize chess game
    context->getChessGame()->reset();
    logger.info("Game started");

    // Build response for the player who started
    json start_response = {
        {"type", "game_started"},
        {"status", context->getStatusMessage()}
    };

    // Broadcast game_started to ALL players
    json game_started_broadcast = {
        {"type", "game_started"},
        {"status", context->getStatusMessage()},
        {"white_player", context->getWhitePlayer()},
        {"black_player", context->getBlackPlayer()}
    };
    context->setPendingBroadcast(game_started_broadcast);

    return start_response;
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
        {"from", strike_data->case_src},         
        {"to", strike_data->case_dest},
        {"piece", strike_data->piece},  
        {"strike_number", strike_data->strike_number},  
        {"capture", strike_data->is_capture},
        {"check", strike_data->is_check},        
        {"checkmate", strike_data->is_checkmate},
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