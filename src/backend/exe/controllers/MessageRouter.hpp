#pragma once

#include <nlohmann/json.hpp>
#include <string>

/**
 * @brief Routes JSON messages to appropriate handlers
 *
 * The controller layer extracts commands from messages and delegates them
 * to model layer (parser, game logic, etc.)
 */
class MessageRouter {
   public:
    MessageRouter() = default;
    ~MessageRouter() = default;

    /**
     * @brief Route a JSON message to the appropriate handler
     * @param msg The parsed JSON message
     * @return JSON response as string (empty if no response needed)
     */
    std::string route(const nlohmann::json& msg);

   private:
    /**
     * @brief Handle a move command
     * @param move The move string
     * @return JSON response as string
     */
    std::string handleMove(const std::string& move);
};