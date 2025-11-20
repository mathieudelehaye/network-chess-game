#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include "Logger.hpp"

using json = nlohmann::json;

class GameContext;  // Forward declaration to avoid circular dependencies

// Abstract State Interface
class IGameState {
   public:
    virtual ~IGameState() = default;

    // State-specific behavior
    virtual json handleJoinRequest(GameContext* context, const std::string& player_id,
                                   const std::string& color) = 0;
    virtual json handleStartRequest(GameContext* context, const std::string& player_id) = 0;
    virtual json handleMoveRequest(GameContext* context, const std::string& player_id,
                                   const std::string& from, const std::string& to) = 0;
    virtual json handleEndRequest(GameContext* context, const std::string& player_id) = 0;
    virtual json handleDisplayBoard(GameContext* context) = 0;

    // Query state
    virtual std::string getStateName() const = 0;
    virtual bool canJoin() const = 0;
    virtual bool canStart() const = 0;
    virtual bool canMove() const = 0;
};