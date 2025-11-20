#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "ChessGame.hpp"
#include "IGameState.hpp"

/**
 * @brief Manages game session state and transitions
 *
 * Owns the ChessGame instance and coordinates state transitions
 */
class GameContext {
   public:
    GameContext();
    ~GameContext() = default;

    using BroadcastCallback = std::function<void(
        const std::string& originating_session_id,
        const json& message, 
        bool to_all
    )>;

    void setBroadcastCallback(BroadcastCallback callback) {
        broadcast_callback_ = std::move(callback);
    }

    void broadcastToAll(const std::string& session_id, const json& message) {
        if (broadcast_callback_) {
            broadcast_callback_(session_id, message, true);
        }
    }

    void broadcastToOthers(const std::string& session_id, const json& message) {
        if (broadcast_callback_) {
            broadcast_callback_(session_id, message, false);
        }
    }

    json resetGame(const std::string& player_id);

    // State transition (used by states to transition themselves)
    void transitionTo(std::unique_ptr<IGameState> state);

    // Player management (public interface for states)
    void setWhitePlayer(const std::string& id) { white_player_id_ = id; }
    void setBlackPlayer(const std::string& id) { black_player_id_ = id; }

    std::string getWhitePlayer() const { return white_player_id_; }
    std::string getBlackPlayer() const { return black_player_id_; }

    bool hasWhitePlayer() const { return !white_player_id_.empty(); }
    bool hasBlackPlayer() const { return !black_player_id_.empty(); }
    bool bothPlayersJoined() const { return hasWhitePlayer() && hasBlackPlayer(); }

    // Status
    std::string getStatusMessage() const;

    // Game access (public interface for states)
    ChessGame* getChessGame() { return chess_game_.get(); }
    const ChessGame* getChessGame() const { return chess_game_.get(); }

    // Request handlers (delegate to current state)
    nlohmann::json handleJoinRequest(const std::string& player_id, const std::string& color);
    nlohmann::json handleStartRequest(const std::string& player_id);
    nlohmann::json handleMoveRequest(const std::string& player_id, const std::string& from,
                                     const std::string& to);
    nlohmann::json handleEndRequest(const std::string& player_id);
    nlohmann::json handleDisplayBoard();

   private:
    std::unique_ptr<IGameState> current_state_;
    std::unique_ptr<ChessGame> chess_game_;
    std::string white_player_id_;
    std::string black_player_id_;
    BroadcastCallback broadcast_callback_;
};