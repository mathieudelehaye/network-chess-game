/**
 * @file GameState.hpp
 * @brief Game state machine implementations for chess game lifecycle.
 *
 * Defines concrete state classes implementing IGameState interface.
 * States: WaitingForPlayers, ReadyToStart, InProgress, GameOver.
 */

#pragma once

#include "IGameState.hpp"

/**
 * @class WaitingForPlayersState
 * @brief Initial state waiting for player join requests.
 *
 * Accepts join requests until both players connected.
 * Transitions to ReadyToStart when both players joined.
 */
class WaitingForPlayersState : public IGameState {
   public:
    /**
     * @brief Handle player join request.
     * @param context Game context
     * @param player_id Joining player's session ID
     * @param color Requested color ("white" or "black")
     * @return JSON response with join result
     */
    json handleJoinRequest(GameContext* context, const std::string& player_id,
                           const std::string& color) override;

    /**
     * @brief Handle single-player join request.
     * @param context Game context
     * @param player_id Joining player's session ID
     * @return JSON response with join result
     */
    json handleJoinRequestAsSinglePlayer(GameContext* context,
                                         const std::string& player_id) override;

    // Parameter name hidden to avoid unused parameter warnings
    json handleStartRequest(GameContext* /*context*/, const std::string& /*player_id*/) override {
        return buildError("Cannot start: waiting for players");
    }

    json handleMoveRequest(GameContext* context, const std::string& player_id, const ParsedMove& move) override {
        return buildError("Cannot move: game not started");
    }

    json handleEndRequest(GameContext* /*context*/, const std::string& /*player_id*/) override {
        return buildError("No game to end");
    }

    json handleDisplayBoard(GameContext* /*context*/) override {
        return buildError("No game to display");
    }

    std::string getStateName() const override { return "WaitingForPlayers"; }
    bool canJoin() const override { return true; }
    bool canStart() const override { return false; }
    bool canMove() const override { return false; }

   private:
    json buildError(const std::string& msg) const {
        return json{{"type", "error"}, {"error", msg}};
    }
};

/**
 * @class ReadyToStartState
 * @brief State when both players joined, waiting for start command.
 *
 * Accepts start request from either player.
 * Transitions to InProgress when game started.
 */
class ReadyToStartState : public IGameState {
   public:
    /**
     * @brief Reject join request (game full).
     * @return JSON error response
     */
    json handleJoinRequest(GameContext* /*context*/, const std::string& /*player_id*/,
                           const std::string& /*color*/) override {
        return buildError("Both players already joined");
    }

    /**
     * @brief Reject single-player join request.
     * @return JSON error response
     */
    json handleJoinRequestAsSinglePlayer(GameContext* /*context*/,
                                         const std::string& /*player_id*/) override {
        return buildError("Game already in progress");
    }

    /**
     * @brief Handle game start request.
     * @param context Game context
     * @param player_id Requesting player's session ID
     * @return JSON response with start result
     */
    json handleStartRequest(GameContext* context, const std::string& player_id) override;

    /**
     * @brief Reject move request (game not started).
     * @return JSON error response
     */
    json handleMoveRequest(GameContext* /*context*/, const std::string& /*player_id*/, const ParsedMove& /*move*/) override {
        return buildError("Game not started yet");
    }

    /**
     * @brief Handle end/reset request.
     * @param player_id Requesting player's session ID
     * @return JSON response with reset result
     */
    json handleEndRequest(GameContext* /*context*/, const std::string& player_id) override;

    /**
     * @brief Reject board display (game not started).
     * @return JSON error response
     */
    json handleDisplayBoard(GameContext* /*context*/) override {
        return buildError("Game not started yet");
    }

    std::string getStateName() const override { return "ReadyToStart"; }
    bool canJoin() const override { return false; }
    bool canStart() const override { return true; }
    bool canMove() const override { return false; }

   private:
    json buildError(const std::string& msg) const {
        return json{{"type", "error"}, {"error", msg}};
    }
};

/**
 * @class InProgressState
 * @brief Active game state handling move requests.
 *
 * Accepts move and display board requests.
 * Transitions to GameOver when game ends (checkmate/stalemate).
 */
class InProgressState : public IGameState {
   public:
    /**
     * @brief Reject join request (game in progress).
     * @return JSON error response
     */
    json handleJoinRequest(GameContext* /*context*/, const std::string& /*player_id*/,
                           const std::string& /*color*/) override {
        return buildError("Game already in progress");
    }

    /**
     * @brief Reject single-player join request.
     * @return JSON error response
     */
    json handleJoinRequestAsSinglePlayer(GameContext* /*context*/,
                                         const std::string& /*player_id*/) override {
        return buildError("Game already in progress");
    }

    /**
     * @brief Reject start request (game already started).
     * @return JSON error response
     */
    json handleStartRequest(GameContext* /*context*/, const std::string& /*player_id*/) override {
        return buildError("Game already started");
    }

    /**
     * @brief Handle move request - validates and executes move.
     * @param context Game context
     * @param move Parsed move from player
     * @return JSON response with move result
     */
    json handleMoveRequest(GameContext* context, const std::string& /*player_id*/, const ParsedMove& /*move*/) override;

    /**
     * @brief Handle end/reset request.
     * @param context Game context
     * @param player_id Requesting player's session ID
     * @return JSON response with reset result
     */
    json handleEndRequest(GameContext* context, const std::string& player_id) override;

    /**
     * @brief Handle board display request.
     * @param context Game context
     * @return JSON response with board state
     */
    json handleDisplayBoard(GameContext* context) override;

    std::string getStateName() const override { return "InProgress"; }
    bool canJoin() const override { return false; }
    bool canStart() const override { return false; }
    bool canMove() const override { return true; }

   private:
    json buildError(const std::string& msg) const {
        return json{{"type", "error"}, {"error", msg}};
    }
};

/**
 * @class GameOverState
 * @brief Final state after game ends.
 *
 * Rejects all requests except reset.
 * Transitions to WaitingForPlayers on reset.
 */
class GameOverState : public IGameState {
   public:
    json handleJoinRequest(GameContext* /*context*/, const std::string& /*player_id*/,
                           const std::string& /*color*/) override {
        return buildError("Game is over. Start a new game");
    }

    json handleJoinRequestAsSinglePlayer(GameContext* /*context*/,
                                         const std::string& /*player_id*/) override {
        return buildError("Game already in progress");
    }

    json handleStartRequest(GameContext* /*context*/, const std::string& /*player_id*/) override {
        return buildError("Game is over. Reset first");
    }

    json handleMoveRequest(GameContext* /*context*/, const std::string& /*player_id*/, const ParsedMove& /*move*/) override {
        return buildError("Game is over");
    }

    json handleEndRequest(GameContext* context, const std::string& player_id) override;

    json handleDisplayBoard(GameContext* /*context*/) override {
        return buildError("Game is over. Start a new game");
    }

    std::string getStateName() const override { return "GameOver"; }
    bool canJoin() const override { return false; }
    bool canStart() const override { return false; }
    bool canMove() const override { return false; }

   private:
    json buildError(const std::string& msg) const {
        return json{{"type", "error"}, {"error", msg}};
    }
};