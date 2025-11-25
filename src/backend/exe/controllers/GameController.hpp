/**
 * @file GameController.hpp
 * @brief Game controller routing messages to model handlers.
 *
 * Parses JSON messages, delegates to GameContext state machine,
 * handles file uploads for game playback.
 */

#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

#include "GameContext.hpp"
#include "Logger.hpp"

/**
 * @struct FileUploadState
 * @brief Tracks file upload progress per session.
 */
struct FileUploadState {
    std::string filename;          ///< Uploaded file name
    int total_size = 0;            ///< Total file size in bytes
    int chunks_total = 0;          ///< Total number of chunks
    int chunks_received = 0;       ///< Chunks received so far
    std::string accumulated_data;  ///< Accumulated file data
};

/**
 * @class GameController
 * @brief Controller routing application messages to model handlers.
 *
 * Parses JSON application messages and delegates to GameContext state machine.
 * Handles file uploads for game playback mode.
 */

class GameController {
   public:
    /**
     * @brief Construct game controller with parser type.
     * @param parser Parser type for game notation
     */
    GameController(ParserType parser);
    
    /**
     * @brief Destructor.
     */
    ~GameController() = default;

    /**
     * @brief Route message to appropriate handler.
     * @param content Raw message string (should be JSON)
     * @param session_id Unique identifier for client session
     * @return JSON response string (optional)
     */
    std::optional<std::string> routeMessage(const std::string& content,
                                            const std::string& session_id);

    /**
     * @brief Route disconnect event.
     * @param session_id Session ID of disconnected client
     */
    void routeDisconnect(const std::string& session_id);

    /**
     * @brief Set callbacks for message routing.
     * @param unicast Callback for unicast messages
     * @param broadcast Callback for broadcast messages
     */
    void setSendCallbacks(UnicastCallback unicast, BroadcastCallback broadcast);

   private:
    /**
     * @brief Handle message from session.
     * @param session_id Client session ID
     * @param message Message content
     * @return JSON response (optional)
     */
    std::optional<std::string> handleMessage(const std::string& session_id, const std::string& message);

    /**
     * @brief Handle join_game command.
     * @param session_id Client session ID
     * @param single_player True if single player mode
     * @param color Joining player color ("white" or "black")
     * @return JSON response
     */
    std::string handleJoinGame(const std::string& session_id, bool single_player,
                               const std::string& color);

    /**
     * @brief Handle start_game command.
     * @param session_id Client session ID
     * @return JSON response
     */
    std::string handleStartGame(const std::string& session_id);

    /**
     * @brief Handle make_move command with unparsed move.
     * @param session_id Client session ID
     * @param move Move string to parse
     * @return JSON response
     */
    std::string handleMoveToParse(const std::string& session_id, const std::string& move);

    /**
     * @brief Handle parsed move.
     * @param session_id Client session ID
     * @param move Parsed move (simple or SAN notation)
     * @return JSON response
     */
    std::string handleParsedMove(const std::string& session_id, const ParsedMove& move);

    /**
     * @brief Handle end_game command.
     * @param session_id Client session ID
     * @return JSON response
     */
    std::string handleEndGame(const std::string& session_id);

    /**
     * @brief Handle display_board command.
     * @return JSON response
     */
    std::string handleDisplayBoard();

    /**
     * @brief Handle file upload chunk.
     * @param msg Parsed JSON message with chunk data
     * @param session_id Client session ID
     * @return JSON response (optional)
     */
    std::optional<std::string> handleFileUploadChunk(const nlohmann::json& msg,
                                                     const std::string& session_id);

    /**
     * @brief Process complete uploaded file content.
     * @param session_id Client session ID
     * @param filename Uploaded filename
     * @param data Complete file content
     */
    void processFileContent(const std::string& session_id, const std::string& filename,
                            const std::string& data);

    std::unique_ptr<GameContext> game_context_;                         ///< Game state machine
    std::unordered_map<std::string, FileUploadState> file_uploads_;     ///< File upload tracking
    std::unique_ptr<IGameParser> parser_;                               ///< Game notation parser
    Logger& logger_;                                                    ///< Logger instance
};