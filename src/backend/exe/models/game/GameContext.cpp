#include "GameContext.hpp"

#include "GameState.hpp"
#include "Logger.hpp"

GameContext::GameContext()
    : current_state_(std::make_unique<WaitingForPlayersState>()),
      chess_game_(std::make_unique<ChessGame>()) {
    auto& logger = Logger::instance();
    logger.info("GameContext initialised");
}

void GameContext::setSendCallbacks(UnicastCallback unicast, BroadcastCallback broadcast) {
    unicast_callback_ = std::move(unicast);
    broadcast_callback_ = std::move(broadcast);
}

void GameContext::unicast(const std::string& session_id, const std::string& message) {
    if (unicast_callback_) {
        unicast_callback_(session_id, message);
    }
}

void GameContext::broadcastToAll(const std::string& session_id, const std::string& message) {
    if (broadcast_callback_) {
        broadcast_callback_(session_id, message, true);
    }
}

void GameContext::broadcastToOthers(const std::string& session_id, const std::string& message) {
    if (broadcast_callback_) {
        broadcast_callback_(session_id, message, false);
    }
}

json GameContext::resetGame(const std::string& player_id) {
    auto& logger = Logger::instance();
    logger.info("Game reset requested by: " + (player_id.empty() ? "system" : player_id));

    // Clear players
    setWhitePlayer("");
    setBlackPlayer("");

    // Reset chess game state
    if (chess_game_) {
        chess_game_->reset();
    }

    // Reset and go back to waiting
    transitionTo(std::make_unique<WaitingForPlayersState>());

    return json{{"type", "game_reset"}, {"status", "Waiting for new players"}};

    // Build response for the player who ended
    json end_response = {
        {"type", "game_reset"},
        {"status", getStatusMessage()},
    };

    // Broadcast game_reset to other players
    json game_end_broadcast = {{"type", "game_reset"},
                               {"status", getStatusMessage()},
                               {"white_player", getWhitePlayer()},
                               {"black_player", getBlackPlayer()}};
    broadcastToOthers(player_id, game_end_broadcast.dump());

    return end_response;
}

void GameContext::transitionTo(std::unique_ptr<IGameState> new_state) {
    std::string old_state = current_state_->getStateName();
    current_state_ = std::move(new_state);
    std::string new_state_name = current_state_->getStateName();

    auto& logger = Logger::instance();
    logger.debug("State transition: " + old_state + " -> " + new_state_name);
}

json GameContext::handleJoinRequest(const std::string& player_id, const std::string& color) {
    return current_state_->handleJoinRequest(this, player_id, color);
}

json GameContext::handleJoinRequestAsSinglePlayer(const std::string& player_id) {
    return current_state_->handleJoinRequestAsSinglePlayer(this, player_id);
}

json GameContext::handleStartRequest(const std::string& player_id) {
    return current_state_->handleStartRequest(this, player_id);
}

json GameContext::handleMoveRequest(const std::string& player_id, const ParsedMove& move) {
    return current_state_->handleMoveRequest(this, player_id, move);
}

json GameContext::handleEndRequest(const std::string& player_id) {
    return current_state_->handleEndRequest(this, player_id);
}

json GameContext::handleDisplayBoard() {
    return current_state_->handleDisplayBoard(this);
}

std::string GameContext::getStatusMessage() const {
    std::string state_name = current_state_->getStateName();

    if (state_name == "WaitingForPlayers") {
        if (hasWhitePlayer() && !hasBlackPlayer()) {
            return "Player 1 (White) joined. Waiting for Player 2 (Black)";
        }
        if (hasBlackPlayer() && !hasWhitePlayer()) {
            return "Player 1 (Black) joined. Waiting for Player 2 (White)";
        }
        return "Waiting for players to join";
    }

    if (state_name == "ReadyToStart") {
        return "Ready to start. Wait for start command to be sent";
    }

    if (state_name == "InProgress") {
        // Query the chess game model for whose turn it is
        if (chess_game_) {
            auto current_player = chess_game_->getCurrentPlayer();
            return current_player == chess::Color::WHITE ? "Game in progress - White's turn"
                                                         : "Game in progress - Black's turn";
        }
        return "Game in progress";
    }

    return "Game over";
}