#include "GameState.hpp"

#include "Logger.hpp"

json WaitingForPlayersState::handleJoinRequest(GameContext* context, const std::string& player_id, const std::string& color) {
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
    
    return json{
        {"type", "join_success"},
        {"color", color},
        {"status", context->getStatusMessage()},
        {"single_player", is_single_player}
    };
}

// ReadyToStartState::handleStartRequest
json ReadyToStartState::handleStartRequest(GameContext* context, const std::string& player_id) {
    auto& logger = Logger::instance();
    
    if (player_id != context->getWhitePlayer() && player_id != context->getBlackPlayer()) {
        return buildError("Only joined players can start the game");
    }
    
    // Reset game instead of handleNewGame()
    context->getGameController()->startNewGame();  // ← Fix this
    
    context->transitionTo(std::make_unique<InProgressState>());
    
    logger.info("Game started");
    
    return json{
        {"type", "game_started"},
        {"status", context->getStatusMessage()}
    };
}

// ReadyToStartState::handleEndRequest
json ReadyToStartState::handleEndRequest(GameContext* context, const std::string& player_id) {
    // Reset and go back to waiting
    context->transitionTo(std::make_unique<WaitingForPlayersState>());
    
    return json{
        {"type", "game_reset"},
        {"status", "Waiting for new players"}
    };
}

// InProgressState::handleMoveRequest
json InProgressState::handleMoveRequest(GameContext* context, const std::string& player_id, const std::string& from, const std::string& to) {
    // Check if it's player's turn
    PlayerColor player_color = PlayerColor::NONE;
    if (player_id == context->getWhitePlayer()) {
        player_color = PlayerColor::WHITE;
    } else if (player_id == context->getBlackPlayer()) {
        player_color = PlayerColor::BLACK;
    } else {
        return buildError("You are not in this game");
    }
    
    if (player_color != context->getCurrentTurn()) {
        return buildError("Not your turn");
    }
    
    // Apply move
    std::string move_result = context->getGameController()->handleMove(from, to);
    json move_json = json::parse(move_result);
    
    if (move_json["type"] == "error") {
        return move_json;
    }
    
    // Toggle turn
    context->toggleTurn();
    
    // Check for game end
    if (move_json["data"]["is_checkmate"].get<bool>()) {
        context->transitionTo(std::make_unique<GameOverState>());
        move_json["status"] = "Checkmate! Game over";
    } else if (move_json["data"]["is_stalemate"].get<bool>()) {
        context->transitionTo(std::make_unique<GameOverState>());
        move_json["status"] = "Stalemate! Game over";
    } else {
        move_json["status"] = context->getStatusMessage();
    }
    
    return move_json;
}

// InProgressState::handleEndRequest
json InProgressState::handleEndRequest(GameContext* context, const std::string& player_id) {
    context->transitionTo(std::make_unique<GameOverState>());
    
    return json{
        {"type", "game_ended"},
        {"status", "Game ended by player"}
    };
}

// GameOverState::handleEndRequest
json GameOverState::handleEndRequest(GameContext* context, const std::string& player_id) {
    // Reset to waiting
    context->transitionTo(std::make_unique<WaitingForPlayersState>());
    
    return json{
        {"type", "game_reset"},
        {"status", "Ready for new game"}
    };
}