#pragma once

#include <memory>
#include <string>
#include <nlohmann/json.hpp>

#include "IGameState.hpp"
#include "GameController.hpp"  // Forward declare instead
#include "Logger.hpp"  // ← Add this

using json = nlohmann::json;

enum class PlayerColor {
    NONE,
    WHITE,
    BLACK
};

class GameController;  // Forward declaration

class GameContext {
public:
    GameContext();
    
    void transitionTo(std::unique_ptr<IGameState> new_state);
    
    json handleJoinRequest(const std::string& player_id, const std::string& color);
    json handleStartRequest(const std::string& player_id);
    json handleMoveRequest(const std::string& player_id, const std::string& from, const std::string& to);
    json handleEndRequest(const std::string& player_id);
    
    IGameState* getCurrentState() const { return current_state_.get(); }
    std::string getStatusMessage() const;
    
    void setWhitePlayer(const std::string& player_id) { white_player_id_ = player_id; }
    void setBlackPlayer(const std::string& player_id) { black_player_id_ = player_id; }
    std::string getWhitePlayer() const { return white_player_id_; }
    std::string getBlackPlayer() const { return black_player_id_; }
    
    bool hasWhitePlayer() const { return !white_player_id_.empty(); }
    bool hasBlackPlayer() const { return !black_player_id_.empty(); }
    bool bothPlayersJoined() const { return hasWhitePlayer() && hasBlackPlayer(); }
    
    PlayerColor getCurrentTurn() const { return current_turn_; }
    void toggleTurn() {
        current_turn_ = (current_turn_ == PlayerColor::WHITE) ? PlayerColor::BLACK : PlayerColor::WHITE;
    }
    
    GameController* getGameController() { return game_controller_.get(); }

private:
    std::unique_ptr<IGameState> current_state_;
    std::unique_ptr<GameController> game_controller_;
    
    std::string white_player_id_;
    std::string black_player_id_;
    PlayerColor current_turn_ = PlayerColor::WHITE;
    
    Logger& logger_;
};