#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

#include "GameContext.hpp"

/**
 * @brief Routes JSON messages to appropriate GameContext handlers
 *
 * All commands (single-player, multiplayer, file uploads) go through
 * the State Pattern FSM in GameContext
 */
class MessageRouter {
public:
    MessageRouter();
    ~MessageRouter() = default;

    /**
     * @brief Route a JSON message to the appropriate handler
     * @param msg The parsed JSON message
     * @param session_id Unique identifier for the client session
     * @return JSON response as string
     */
    std::string route(const nlohmann::json& msg, const std::string& session_id);

private:
    /**
     * @brief Handle join_game command
     * @param session_id Client session ID
     * @param color "white" or "black"
     * @return JSON response
     */
    std::string handleJoinGame(const std::string& session_id, const std::string& color);
    
    /**
     * @brief Handle start_game command
     * @param session_id Client session ID
     * @return JSON response
     */
    std::string handleStartGame(const std::string& session_id);
    
    /**
     * @brief Handle make_move command
     * @param session_id Client session ID
     * @param from Starting square (e.g., "e2")
     * @param to Target square (e.g., "e4")
     * @return JSON response
     */
    std::string handleMakeMove(const std::string& session_id, 
                               const std::string& from, 
                               const std::string& to);
    
    /**
     * @brief Handle end_game command
     * @param session_id Client session ID
     * @return JSON response
     */
    std::string handleEndGame(const std::string& session_id);
    
    /**
     * @brief Handle get_status command
     * @return JSON response with game state
     */
    std::string handleGetStatus();
    
    /**
     * @brief Handle display_board command
     * @return JSON response
     */
    std::string handleDisplayBoard();
    
    /**
     * @brief Handle upload_game command (file upload)
     * @param content Game file content
     * @param filename Original filename
     * @return JSON response
     */
    std::string handleUploadGame(const std::string& content, const std::string& filename);
    
    /**
     * @brief Parse and handle simple move notation (e.g., "e2-e4")
     * @param move The move string
     * @return JSON response
     */
    std::string handleSimpleMove(const std::string& move);

    // State machine for all game modes
    std::unique_ptr<GameContext> game_context_;
};