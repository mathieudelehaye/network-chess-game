#include "GameContext.hpp"
#include "GameState.hpp"
#include "GameController.hpp"  // ← Include here
#include "Logger.hpp"

GameContext::GameContext() 
    : current_state_(std::make_unique<WaitingForPlayersState>()),
      game_controller_(std::make_unique<GameController>()),
      logger_(Logger::instance()) {
    logger_.info("GameContext initialized");
}

void GameContext::transitionTo(std::unique_ptr<IGameState> new_state) {
    std::string old_state = current_state_->getStateName();
    current_state_ = std::move(new_state);
    std::string new_state_name = current_state_->getStateName();
    
    logger_.info("State transition: " + old_state + " -> " + new_state_name);
}

json GameContext::handleJoinRequest(const std::string& player_id, const std::string& color) {
    return current_state_->handleJoinRequest(this, player_id, color);
}

json GameContext::handleStartRequest(const std::string& player_id) {
    return current_state_->handleStartRequest(this, player_id);
}

json GameContext::handleMoveRequest(const std::string& player_id, const std::string& from, const std::string& to) {
    return current_state_->handleMoveRequest(this, player_id, from, to);
}

json GameContext::handleEndRequest(const std::string& player_id) {
    return current_state_->handleEndRequest(this, player_id);
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
        return "Both players joined. Wait for start command to be sent by a player";
    }
    
    if (state_name == "InProgress") {
        return current_turn_ == PlayerColor::WHITE ? 
            "Game in progress - White's turn" : 
            "Game in progress - Black's turn";
    }
    
    return "Game over";
}