#include "GameController.hpp"

#include <utility>

#include "GameContext.hpp"
#include "MoveParser.hpp"
#include "ParserFactory.hpp"

using json = nlohmann::json;

GameController::GameController(ParserType parser)
    : game_context_(std::make_unique<GameContext>()),
      parser_(std::move(ParserFactory::createParser(parser))),
      logger_(Logger::instance()) {
    logger_.debug("GameController initialised");
}

void GameController::setSendCallbacks(UnicastCallback unicast, BroadcastCallback broadcast) {
    if (game_context_) {
        game_context_->setSendCallbacks(std::move(unicast), std::move(broadcast));
    }
}

std::optional<std::string> GameController::routeMessage(const std::string& message,
                                                        const std::string& session_id) {
    // Parse application message (should be JSON)
    try {
        return handleMessage(session_id, message);

    } catch (const json::parse_error& e) {
        logger_.error("JSON parse error: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Invalid JSON format";
        error["details"] = e.what();
        return error.dump();

    } catch (const json::exception& e) {
        logger_.error("JSON error: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Invalid message structure";
        return error.dump();

    } catch (const std::exception& e) {
        logger_.critical("Unexpected error: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Internal server error";
        return error.dump();
    }
}

void GameController::routeDisconnect(const std::string& session_id) {
    // Detect if one of the players disconnected and if so reset the game

    logger_.debug("Handling disconnect for session: " + session_id);

    std::string disconnected_color;

    // Only reset the game if disconnected player had joined it.
    bool hadPlayerJoined = false;

    // Thread-safe instruction block
    {
        std::lock_guard<std::mutex> lock(game_context_->getMutex());

        // Check if this session was a player
        if (game_context_->getWhitePlayer() == session_id) {
            disconnected_color = "white";
            game_context_->setWhitePlayer("");  // Clear white player
            hadPlayerJoined = true;
        } else if (game_context_->getBlackPlayer() == session_id) {
            disconnected_color = "black";
            game_context_->setBlackPlayer("");  // Clear black player
            hadPlayerJoined = true;
        }

        logger_.info(disconnected_color + " player disconnected");

        if (hadPlayerJoined) {
            logger_.info("Resetting game");
            game_context_->resetGame("");
        }
    }

    if (hadPlayerJoined) {
        if (!disconnected_color.empty()) {
            logger_.debug("Notifying other players that game reset");

            // Broadcast reset to other players
            json reset_broadcast = {{"type", "game_reset"},
                                    {"reason", "all_players_disconnected"},
                                    {"status", "Waiting for players..."}};
            game_context_->broadcastToOthers(session_id, reset_broadcast.dump());
        }
    }
}

std::optional<std::string> GameController::handleMessage(const std::string& session_id,
                                                         const std::string& message) {
    logger_.debug("Routing message for session: " + session_id);

    auto json_message = json::parse(message);

    // Command-based routing
    if (json_message.contains("command")) {
        std::string command = json_message["command"];

        if (command == "upload_game") {
            return handleFileUploadChunk(json_message, session_id);
        } else if (command == "join_game") {
            return handleJoinGame(session_id, json_message["single_player"], json_message["color"]);
        } else if (command == "start_game") {
            return handleStartGame(session_id);
        } else if (command == "make_move") {
            return handleMoveToParse(session_id, json_message["move"]);
        } else if (command == "end_game") {
            return handleEndGame(session_id);
        } else if (command == "display_board") {
            return handleDisplayBoard();
        }
    }

    logger_.warning("Unknown message type");
    json error;
    error["error"] = "Unknown command";
    return error.dump();
}

std::string GameController::handleJoinGame(const std::string& session_id, bool single_player,
                                           const std::string& color) {
    logger_.info("Session " + session_id + " joining as " + color);

    json response;

    if (single_player) {
        std::lock_guard<std::mutex> lock(game_context_->getMutex());
        response = game_context_->handleJoinRequestAsSinglePlayer(session_id);
    } else {
        std::lock_guard<std::mutex> lock(game_context_->getMutex());
        response = game_context_->handleJoinRequest(session_id, color);
    }

    return response.dump();
}

std::string GameController::handleStartGame(const std::string& session_id) {
    logger_.info("Session " + session_id + " starting game");

    json response;

    // Thread-safe instruction block
    {
        std::lock_guard<std::mutex> lock(game_context_->getMutex());
        response = game_context_->handleStartRequest(session_id);
    }

    return response.dump();
}

std::string GameController::handleMoveToParse(const std::string& session_id,
                                              const std::string& move) {
    logger_.debug("Session " + session_id + " parsing move with " + parser_->getParserType() +
                  ": " + move);

    auto parsed_move = parser_->parseMove(move);

    if (!parsed_move) {
        json error;
        error["type"] = "error";
        error["error"] = "Couldn't parse move using " + parser_->getParserType();
        error["parser_used"] = parser_->getParserType();
        return error.dump();
    }

    return handleParsedMove(session_id, *parsed_move);
}

std::string GameController::handleParsedMove(const std::string& session_id,
                                             const ParsedMove& move) {
    if (move.is_san) {
        logger_.info("Session " + session_id + " move: " + move.notation);
    } else {
        logger_.info("Session " + session_id + " move: " + move.from + "-" + move.to);
    }

    json response;

    // Thread-safe instruction block
    {
        std::lock_guard<std::mutex> lock(game_context_->getMutex());
        response = game_context_->handleMoveRequest(session_id, move);
    }

    return response.dump();
}

std::string GameController::handleEndGame(const std::string& session_id) {
    logger_.info("Session " + session_id + " ending game");

    json response;

    // Thread-safe instruction block
    {
        std::lock_guard<std::mutex> lock(game_context_->getMutex());
        response = game_context_->handleEndRequest(session_id);
    }

    return response.dump();
}

std::string GameController::handleDisplayBoard() {
    logger_.debug("Displaying board");

    json response;

    // Thread-safe instruction block
    {
        std::lock_guard<std::mutex> lock(game_context_->getMutex());
        response = game_context_->handleDisplayBoard();
    }

    return response.dump();
}

// TODO: file chunk upload and file reconstruction should be moved to a separate
// class in the Utils part of the backend source code.
std::optional<std::string> GameController::handleFileUploadChunk(const nlohmann::json& json_message,
                                                                 const std::string& session_id) {
    try {
        auto metadata = json_message["metadata"];
        std::string filename = metadata["filename"];
        int total_size = metadata["total_size"];
        int chunks_total = metadata["chunks_total"];
        int chunk_current = metadata["chunk_current"];
        std::string chunk_data = json_message["data"];

        std::string upload_key = session_id + ":" + filename;
        auto& upload = file_uploads_[upload_key];

        if (chunk_current == 1) {
            upload.filename = filename;
            upload.total_size = total_size;
            upload.chunks_total = chunks_total;
            upload.chunks_received = 0;
            upload.accumulated_data.clear();
            upload.accumulated_data.reserve(total_size);

            logger_.info("Starting file upload: " + filename + " (" + std::to_string(total_size) +
                         " bytes) for session " + session_id);
        }

        upload.accumulated_data += chunk_data;

        // TCP or Unix stream socket IPC guarantee packet order
        upload.chunks_received = chunk_current;

        int percent = (chunk_current * 100) / chunks_total;
        logger_.info("Upload progress " + filename + ": " + std::to_string(percent) + "% (" +
                     std::to_string(chunk_current) + "/" + std::to_string(chunks_total) + ")");

        if (chunk_current >= chunks_total) {
            logger_.info("File upload complete: " + filename);
            processFileContent(session_id, filename, upload.accumulated_data);

            // Clean up upload state
            file_uploads_.erase(upload_key);

            // Return empty string - responses already sent progressively
            return std::nullopt;
        }

        // Return progress acknowledgment for intermediate chunks
        json ack;
        ack["type"] = "upload_progress";
        ack["filename"] = filename;
        ack["chunk_received"] = chunk_current;
        ack["chunks_total"] = chunks_total;
        ack["percent"] = percent;
        return ack.dump();

    } catch (const json::exception& e) {
        logger_.error("Error processing file upload: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Invalid upload chunk format";
        return error.dump();
    }
}

void GameController::processFileContent(const std::string& session_id, const std::string& filename,
                                        const std::string& data) {
    auto moves = parser_->parseGame(data);

    if (!moves.has_value() || (*moves).empty()) {
        logger_.warning("No valid moves found in game file");

        json error_response = {{"type", "game_complete"},
                               {"filename", filename},
                               {"total_moves", 0},
                               {"error", "No valid moves found. Check file format."}};

        game_context_->unicast(session_id, error_response.dump());
        return;
    }

    logger_.info("Parsed " + std::to_string((*moves).size()) + " moves from file");

    // Execute each move and send move_result
    int successful_moves = 0;
    std::string last_error;

    bool game_complete = false;
    std::string result = "";

    for (size_t i = 0; i < (*moves).size(); ++i) {
        const auto& move = (*moves)[i];

        try {
            std::string move_response = handleParsedMove(session_id, move);
            json move_json = json::parse(move_response);

            // Small delay to allow client to process
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            // Check if move failed
            if (move_json.contains("type") && move_json["type"] == "error") {
                logger_.warning("Error at move " + std::to_string(i + 1));
                last_error = move_json.value("error", "Unknown error");
                break;
            }

            // Send move_result immediately to client
            game_context_->unicast(session_id, move_response);

            successful_moves++;

            // Check if game is over
            if (move_json.contains("strike")) {
                auto strike = move_json["strike"];
                bool checkmate = strike.value("checkmate", false);
                bool checkstale = strike.value("stalemate", false);
                if (checkmate || checkstale) {
                    logger_.info("Game ended at move " + std::to_string(i + 1));
                    const std::string winner_color = strike.value("color", "");
                    game_complete = true;
                    result = checkmate ? "checkmate (" + winner_color + " wins)"
                                       : "checkmate (" + winner_color + " wins)";
                    break;
                }
            }

        } catch (const std::exception& e) {
            logger_.error("Exception at move " + std::to_string(i + 1) + ": " + e.what());
            last_error = e.what();
            break;
        }
    }

    if (game_complete) {
        // Send final game_complete message
        json final_response = {{"type", "game_over"},
                               {"result", result},
                               {"filename", filename},
                               {"total_moves", successful_moves},
                               {"requested_moves", (*moves).size()}};

        if (!last_error.empty()) {
            final_response["error"] = last_error;
        }

        game_context_->unicast(session_id, final_response.dump());
    }
}