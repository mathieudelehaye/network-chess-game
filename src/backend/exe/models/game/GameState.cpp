#include "GameState.hpp"

#include "GameContext.hpp"
#include "Logger.hpp"

json WaitingForPlayersState::handleJoinRequest(
    GameContext* context, const std::string& player_id,
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

    if (context->bothPlayersJoined()) {
        context->transitionTo(std::make_unique<ReadyToStartState>());

        logger.info("Both players joined! Ready to start.");

        json ready_broadcast = {{"type", "game_ready"},
                                {"status", "Both players joined. You can now start the game!"},
                                {"white_player", context->getWhitePlayer()},
                                {"black_player", context->getBlackPlayer()}};
        context->broadcastToAll(player_id, ready_broadcast.dump());

    } else {
        // Only one player joined so far Broadcast player_joined to other clients
        json player_joined = {
            {"type", "player_joined"}, {"color", color}, {"status", context->getStatusMessage()}};
        context->broadcastToOthers(player_id, player_joined.dump());
    }

    // Send response for the joining player
    json join_response = {{"type", "join_success"},
                          {"session_id", player_id},
                          {"color", color},
                          {"status", context->getStatusMessage()},
                          {"single_player", false}};

    return join_response;
}

json WaitingForPlayersState::handleJoinRequestAsSinglePlayer(GameContext* context,
                                                             const std::string& player_id) {
    auto& logger = Logger::instance();

    // Single player mode: player plays both colors
    context->setWhitePlayer(player_id);
    context->setBlackPlayer(player_id);
    logger.info("Player " + player_id + " joined as single player");

    logger.info("Single player joined! Ready to start.");

    context->transitionTo(std::make_unique<ReadyToStartState>());

    json join_response = {{"type", "join_success"},
                          {"session_id", player_id},
                          {"status", context->getStatusMessage()},
                          {"single_player", true}};

    // Send response to the single player
    return join_response;
}

json ReadyToStartState::handleStartRequest(GameContext* context, const std::string& player_id) {
    auto& logger = Logger::instance();
    logger.info("Session " + player_id + " starting game");

    context->transitionTo(std::make_unique<InProgressState>());

    // Initialise chess game
    auto* game = context->getChessGame();
    game->reset();
    logger.info("Game started");
    
    // Get initial board state
    std::string fen = game->getFEN();

    // Build response for the player who started
    json start_response = {
        {"type", "game_started"}, 
        {"status", context->getStatusMessage()},
        {"board", {
            {"fen", fen}
        }},
    };

    // Broadcast game_started to ALL players
    json game_started_broadcast = {
        {"type", "game_started"},
        {"status", context->getStatusMessage()},
        {"white_player", context->getWhitePlayer()},
        {"black_player", context->getBlackPlayer()},
        {"board", {
            {"fen", fen}
        }}
    };
    context->broadcastToOthers(player_id, game_started_broadcast.dump());

    return start_response;
}

json ReadyToStartState::handleEndRequest(GameContext* context, const std::string& player_id) {
    return context->resetGame(player_id);
}

json InProgressState::handleMoveRequest(
    GameContext* context, const std::string& player_id,
    const ParsedMove& move) {
        
    auto* game = context->getChessGame();
    if (!game) {
        return buildError("Game not initialised");
    }

    // Apply move to model
    auto strike_data = game->applyMove(move);
    if (!strike_data) {
        return buildError("Invalid move");
    }

    // Get FEN representation
    std::string fen = game->getFEN();

    // Build response
    json response;
    response["type"] = "move_result";
    response["success"] = true;
    response["strike"] = {
        {"case_src", strike_data->case_src},
        {"case_dest", strike_data->case_dest},
        {"piece", strike_data->piece},
        {"color", strike_data->color},
        {"strike_number", strike_data->strike_number},
        {"is_capture", strike_data->is_capture},
        {"captured_piece", strike_data->captured_piece},
        {"captured_color", strike_data->captured_color},
        {"is_castling", strike_data->is_castling},
        {"castling_type", strike_data->castling_type},
        {"check", strike_data->is_check},
        {"checkmate", strike_data->is_checkmate},
        {"stalemate", strike_data->is_stalemate}};
    
    // Add board data with both formats
    response["board"] = {
        {"fen", fen}
    };

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

    // Broadcast move to other players
    context->broadcastToOthers(player_id, response.dump());

    return response;
}

json InProgressState::handleEndRequest(GameContext* context, const std::string& player_id) {
    return context->resetGame(player_id);
}

json InProgressState::handleDisplayBoard(GameContext* context) {
    auto& logger = Logger::instance();
    auto* game = context->getChessGame();

    if (!game) {
        return buildError("Game not initialised");
    }

    try {
        std::string boardASCII = game->getBoardFormatted();
        logger.trace("Received ASCII board:\n" + boardASCII);

        json response;
        response["type"] = "board_display";
        response["status"] = "ok";
        response["data"]["board"] = boardASCII;

        return response;

    } catch (const std::exception& e) {
        logger.error("Failed to display board: " + std::string(e.what()));

        return buildError("Failed to display board");
    }
}

json GameOverState::handleEndRequest(GameContext* context, const std::string& player_id) {
    return context->resetGame(player_id);
}