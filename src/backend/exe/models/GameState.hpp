#pragma once

#include "IGameState.hpp"

// State 1: Waiting for Players
class WaitingForPlayersState : public IGameState {
   public:
    json handleJoinRequest(GameContext* context, const std::string& player_id,
                           const std::string& color) override;

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

// State 2: Ready to Start
class ReadyToStartState : public IGameState {
   public:
    json handleJoinRequest(GameContext* /*context*/, const std::string& /*player_id*/,
                           const std::string& /*color*/) override {
        return buildError("Both players already joined");
    }

    json handleJoinRequestAsSinglePlayer(GameContext* /*context*/,
                                         const std::string& /*player_id*/) override {
        return buildError("Game already in progress");
    }

    json handleStartRequest(GameContext* context, const std::string& player_id) override;

    json handleMoveRequest(GameContext* /*context*/, const std::string& /*player_id*/, const ParsedMove& /*move*/) override {
        return buildError("Game not started yet");
    }

    json handleEndRequest(GameContext* /*context*/, const std::string& player_id) override;

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

// State 3: Game In Progress
class InProgressState : public IGameState {
   public:
    json handleJoinRequest(GameContext* /*context*/, const std::string& /*player_id*/,
                           const std::string& /*color*/) override {
        return buildError("Game already in progress");
    }

    json handleJoinRequestAsSinglePlayer(GameContext* /*context*/,
                                         const std::string& /*player_id*/) override {
        return buildError("Game already in progress");
    }

    json handleStartRequest(GameContext* /*context*/, const std::string& /*player_id*/) override {
        return buildError("Game already started");
    }

    json handleMoveRequest(GameContext* context, const std::string& /*player_id*/, const ParsedMove& /*move*/) override;

    json handleEndRequest(GameContext* context, const std::string& player_id) override;

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

// State 4: Game Over
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