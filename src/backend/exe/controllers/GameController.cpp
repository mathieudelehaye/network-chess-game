#include "GameController.hpp"

#include "GameContext.hpp"
#include "Logger.hpp"
#include "MoveParser.hpp"

using json = nlohmann::json;

GameController::GameController(std::shared_ptr<GameContext> context)
    : game_context_(std::move(context))  // each controller moves a copy of the same game context
{
    auto& logger = Logger::instance();
    logger.debug("GameController initialised");
}

std::string GameController::routeMessage(const std::string& content,
                                         const std::string& session_id) {
    auto& logger = Logger::instance();

    // Parse application message (should be JSON)
    try {
        json msg = json::parse(content);

        logger.debug("Routing message for session: " + session_id);

        // File upload chunks
        if (msg.contains("type") && msg["type"] == "file_upload") {
            return handleFileUploadChunk(msg, session_id);
        }

        // Command-based routing
        if (msg.contains("command")) {
            std::string command = msg["command"];

            if (command == "join_game") {
                return handleJoinGame(session_id, msg["color"]);
            } else if (command == "start_game") {
                return handleStartGame(session_id);
            } else if (command == "make_move") {
                return handleMakeMove(session_id, msg["from"], msg["to"]);
            } else if (command == "end_game") {
                return handleEndGame(session_id);
            } else if (command == "display_board") {
                return handleDisplayBoard();
            }
        }

        logger.warning("Unknown message type");
        json error;
        error["error"] = "Unknown command";
        return error.dump();

    } catch (const json::parse_error& e) {
        logger.error("JSON parse error: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Invalid JSON format";
        error["details"] = e.what();
        return error.dump();

    } catch (const json::exception& e) {
        logger.error("JSON error: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Invalid message structure";
        return error.dump();

    } catch (const std::exception& e) {
        logger.critical("Unexpected error: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Internal server error";
        return error.dump();
    }
}

std::string GameController::handleJoinGame(const std::string& session_id,
                                           const std::string& color) {
    auto& logger = Logger::instance();
    logger.info("Session " + session_id + " joining as " + color);

    json response = game_context_->handleJoinRequest(session_id, color);
    return response.dump();
}

std::string GameController::handleStartGame(const std::string& session_id) {
    auto& logger = Logger::instance();
    logger.info("Session " + session_id + " starting game");

    json response = game_context_->handleStartRequest(session_id);
    return response.dump();
}

std::string GameController::handleMakeMove(const std::string& session_id, const std::string& from,
                                           const std::string& to) {
    auto& logger = Logger::instance();
    logger.info("Session " + session_id + " move: " + from + "-" + to);

    json response = game_context_->handleMoveRequest(session_id, from, to);
    return response.dump();
}

std::string GameController::handleEndGame(const std::string& session_id) {
    auto& logger = Logger::instance();
    logger.info("Session " + session_id + " ending game");

    json response = game_context_->handleEndRequest(session_id);
    return response.dump();
}

std::string GameController::handleDisplayBoard() {
    auto& logger = Logger::instance();
    logger.debug("Displaying board");

    json response = game_context_->handleDisplayBoard();  // ← Delegate to context
    return response.dump();
}

// TODO: file chunk upload and file reconstruction should be moved to a separate
// class in the Utils part of the backend source code.
std::string GameController::handleFileUploadChunk(const nlohmann::json& msg,
                                                  const std::string& session_id) {
    auto& logger = Logger::instance();

    try {
        auto metadata = msg["metadata"];
        std::string filename = metadata["filename"];
        int total_size = metadata["total_size"];
        int chunks_total = metadata["chunks_total"];
        int chunk_current = metadata["chunk_current"];
        std::string chunk_data = msg["data"];

        std::string upload_key = session_id + ":" + filename;
        auto& upload = file_uploads_[upload_key];

        if (chunk_current == 1) {
            upload.filename = filename;
            upload.total_size = total_size;
            upload.chunks_total = chunks_total;
            upload.chunks_received = 0;
            upload.accumulated_data.clear();
            upload.accumulated_data.reserve(total_size);

            logger.info("Starting file upload: " + filename + " (" + std::to_string(total_size) +
                        " bytes) for session " + session_id);
        }

        upload.accumulated_data += chunk_data;
        upload.chunks_received = chunk_current;

        int percent = (chunk_current * 100) / chunks_total;
        logger.info("Upload progress " + filename + ": " + std::to_string(percent) + "% (" +
                    std::to_string(chunk_current) + "/" + std::to_string(chunks_total) + ")");

        json ack;
        ack["type"] = "upload_progress";
        ack["filename"] = filename;
        ack["chunk_received"] = chunk_current;
        ack["chunks_total"] = chunks_total;
        ack["percent"] = percent;

        if (chunk_current >= chunks_total) {
            logger.info("File upload complete: " + filename);

            MoveParser parser;
            auto moves = parser.parseGame(upload.accumulated_data);

            json responses = json::array();
            for (size_t i = 0; i < moves.size(); ++i) {
                const auto& move = moves[i];
                std::string move_response = handleMakeMove(session_id, move.from, move.to);
                json move_json = json::parse(move_response);
                responses.push_back(move_json);

                if (move_json.contains("type") && move_json["type"] == "error") {
                    logger.warning("Error at move " + std::to_string(i + 1));
                    break;
                }
            }

            file_uploads_.erase(upload_key);

            json final_response;
            final_response["type"] = "game_complete";
            final_response["filename"] = filename;
            final_response["total_moves"] = responses.size();
            final_response["moves"] = responses;
            return final_response.dump();
        }

        return ack.dump();

    } catch (const json::exception& e) {
        logger.error("Error processing file upload: " + std::string(e.what()));

        json error;
        error["type"] = "error";
        error["error"] = "Invalid upload chunk format";
        return error.dump();
    }
}