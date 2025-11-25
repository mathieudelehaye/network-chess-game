/**
 * @file GameContext.hpp
 * @brief Game session state management and coordination.
 *
 * Manages game state transitions, player tracking, chess game instance,
 * and message routing via callbacks.
 */

#pragma once

#include <chrono>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "ChessGame.hpp"
#include "IGameState.hpp"
#include "ParserFactory.hpp"

/**
 * @brief Callback to send message to specific session.
 */
using UnicastCallback = std::function<void(const std::string& session_id, const json& message)>;

/**
 * @brief Callback to broadcast message to sessions.
 */
using BroadcastCallback = std::function<void(const std::string& originating_session_id,
                                             const json& message, bool to_all)>;

/**
 * @class GameContext
 * @brief Manages game session state and transitions.
 *
 * Owns the ChessGame instance, coordinates state transitions,
 * tracks players, manages game timer, and provides message routing.
 */
class GameContext {
   public:
    /**
     * @brief Construct game context with initial state.
     */
    GameContext();

    /**
     * @brief Destructor.
     */
    ~GameContext() = default;

    /**
     * @brief Set callbacks for message routing.
     * @param unicast Callback for unicast messages
     * @param broadcast Callback for broadcast messages
     */
    void setSendCallbacks(UnicastCallback unicast, BroadcastCallback broadcast);

    /**
     * @brief Send message to specific session.
     * @param session_id Target session ID
     * @param message Message to send
     */
    void unicast(const std::string& session_id, const std::string& message);

    /**
     * @brief Broadcast message to all sessions.
     * @param session_id Originating session ID
     * @param message Message to broadcast
     */
    void broadcastToAll(const std::string& session_id, const std::string& message);

    /**
     * @brief Broadcast message to all sessions except originator.
     * @param session_id Originating session ID to exclude
     * @param message Message to broadcast
     */
    void broadcastToOthers(const std::string& session_id, const std::string& message);

    /**
     * @brief Reset game to initial state.
     * @param player_id Player requesting reset
     * @return JSON response with reset result
     */
    json resetGame(const std::string& player_id);

    /**
     * @brief Start game timer.
     */
    void startGameTimer();

    /**
     * @brief Get elapsed time since game start.
     * @return Elapsed seconds
     */
    int getElapsedSeconds() const;

    /**
     * @brief Transition to new state.
     * @param state New state instance (ownership transferred)
     */
    void transitionTo(std::unique_ptr<IGameState> state);

    /**
     * @brief Set white player session ID.
     * @param id White player's session ID
     */
    void setWhitePlayer(const std::string& id) { white_player_id_ = id; }

    /**
     * @brief Set black player session ID.
     * @param id Black player's session ID
     */
    void setBlackPlayer(const std::string& id) { black_player_id_ = id; }

    /**
     * @brief Get white player session ID.
     * @return White player's session ID
     */
    std::string getWhitePlayer() const { return white_player_id_; }

    /**
     * @brief Get black player session ID.
     * @return Black player's session ID
     */
    std::string getBlackPlayer() const { return black_player_id_; }

    /**
     * @brief Check if white player joined.
     * @return True if white player assigned
     */
    bool hasWhitePlayer() const { return !white_player_id_.empty(); }

    /**
     * @brief Check if black player joined.
     * @return True if black player assigned
     */
    bool hasBlackPlayer() const { return !black_player_id_.empty(); }

    /**
     * @brief Check if both players joined.
     * @return True if both white and black players assigned
     */
    bool bothPlayersJoined() const { return hasWhitePlayer() && hasBlackPlayer(); }

    /**
     * @brief Get current game status message.
     * @return Status string
     */
    std::string getStatusMessage() const;

    /**
     * @brief Get chess game instance (mutable).
     * @return Pointer to ChessGame
     */
    ChessGame* getChessGame() { return chess_game_.get(); }

    /**
     * @brief Get chess game instance (const).
     * @return Const pointer to ChessGame
     */
    const ChessGame* getChessGame() const { return chess_game_.get(); }

    /**
     * @brief Get mutex for thread-safe access.
     * @return Reference to mutex
     */
    std::mutex& getMutex() { return mutex_; }

    /**
     * @brief Handle join request (delegates to current state).
     * @param player_id Joining player's session ID
     * @param color Requested color
     * @return JSON response
     */
    nlohmann::json handleJoinRequest(const std::string& player_id, const std::string& color);

    /**
     * @brief Handle single-player join request (delegates to current state).
     * @param player_id Joining player's session ID
     * @return JSON response
     */
    nlohmann::json handleJoinRequestAsSinglePlayer(const std::string& player_id);

    /**
     * @brief Handle start request (delegates to current state).
     * @param player_id Requesting player's session ID
     * @return JSON response
     */
    nlohmann::json handleStartRequest(const std::string& player_id);

    /**
     * @brief Handle move request (delegates to current state).
     * @param player_id Moving player's session ID
     * @param move Parsed move
     * @return JSON response
     */
    nlohmann::json handleMoveRequest(const std::string& player_id, const ParsedMove& move);

    /**
     * @brief Handle end/reset request (delegates to current state).
     * @param player_id Requesting player's session ID
     * @return JSON response
     */
    nlohmann::json handleEndRequest(const std::string& player_id);

    /**
     * @brief Handle display board request (delegates to current state).
     * @return JSON response with board state
     */
    nlohmann::json handleDisplayBoard();

   private:
    std::unique_ptr<IGameState> current_state_;
    std::unique_ptr<ChessGame> chess_game_;
    UnicastCallback unicast_callback_;
    BroadcastCallback broadcast_callback_;
    std::string white_player_id_;
    std::string black_player_id_;
    mutable std::mutex mutex_;

    std::chrono::steady_clock::time_point game_start_time_;
    bool timer_started_ = false;
};