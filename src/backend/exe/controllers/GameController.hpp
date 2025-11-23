#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "GameContext.hpp"
#include "Logger.hpp"

struct FileUploadState {
    std::string filename;
    int total_size = 0;
    int chunks_total = 0;
    int chunks_received = 0;
    std::string accumulated_data;
};

/**
 * @brief Controller for the game which routes application messages to
 * appropriate model handlers
 *
 * Parses JSON application messages and delegates to GameContext state machine
 */

class GameController {
   public:
    GameController();
    ~GameController() = default;

    /**
     * @brief Route a message (should be JSON)
     * @param content Raw message as string
     * @param session_id Unique identifier for the client session
     * @return JSON response as string (optional)
     */
    std::optional<std::string> routeMessage(const std::string& content,
                                            const std::string& session_id);

    // Could be an internal message, but it seems more straighforward that way.
    void routeDisconnect(const std::string& session_id);

    void setSendCallbacks(UnicastCallback unicast, BroadcastCallback broadcast);

   private:
    std::optional<std::string> handleMessage(const std::string& session_id, const std::string& message);

    /**
     * @brief Handle join_game command
     * @param session_id Client session ID
     * @param single_player True if single player mode
     * @param color The joining player color ("white" or "black")
     * @return JSON response
     */
    std::string handleJoinGame(const std::string& session_id, bool single_player,
                               const std::string& color);

    /**
     * @brief Handle start_game command
     * @param session_id Client session ID
     * @return JSON response
     */
    std::string handleStartGame(const std::string& session_id);

    /**
     * @brief Handle make_move command
     * @param session_id Client session ID
     * @param move Move to parse
     * @return JSON response
     */
    std::string handleMoveToParse(const std::string& session_id, const std::string& move);

    /**
     * @brief Handle parsed move
     * @param session_id Client session ID
     * @param from Starting square (e.g., "e2")
     * @param to Target square (e.g., "e4")
     * @return JSON response
     */
    std::string handleParsedMove(const std::string& session_id, const std::string& from,
                                 const std::string& to);

    /**
     * @brief Handle end_game command
     * @param session_id Client session ID
     * @return JSON response
     */
    std::string handleEndGame(const std::string& session_id);

    /**
     * @brief Handle display_board command
     * @return JSON response
     */
    std::string handleDisplayBoard();

    /**
     * @brief Handle file upload chunk
     * @param msg Parsed JSON message containing chunk data
     * @param session_id Client session ID
     * @return JSON response
     */
    std::optional<std::string> handleFileUploadChunk(const nlohmann::json& msg,
                                                     const std::string& session_id);

    void processFileContent(const std::string& session_id, const std::string& filename,
                            const std::string& data);

    // State machine for all game modes
    std::unique_ptr<GameContext> game_context_;

    // Track file uploads per session
    std::unordered_map<std::string, FileUploadState> file_uploads_;

    Logger& logger_;
};