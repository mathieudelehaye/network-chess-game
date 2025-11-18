#pragma once

#include <chess.hpp>  // chess library
#include <optional>
#include <string>

#include "GameState.hpp"
#include "StrikeData.hpp"

class ChessGame {
   public:
    ChessGame();
    ~ChessGame() = default;

    /**
     * @brief Apply a move using ParsedMove from MoveParser
     * @param from Source square (e.g., "e2")
     * @param to Destination square (e.g., "e4")
     * @return StrikeData if move is legal, std::nullopt otherwise
     */
    std::optional<StrikeData> applyMove(const std::string& from, const std::string& to);

    /**
     * @brief Get current game state
     */
    GameState getState() const;

    /**
     * @brief Get current player to move
     */
    Player getCurrentPlayer() const;

    /**
     * @brief Get current board as FEN string
     */
    std::string getFEN() const;

    /**
     * @brief Get ASCII art representation of current board
     */
    std::string getBoardASCII() const;

    /**
     * @brief Check if a move is legal (without applying it)
     */
    bool isLegalMove(const std::string& from, const std::string& to) const;

    /**
     * @brief Reset the game
     */
    void reset();

    /**
     * @brief Get current move number
     */
    int getMoveNumber() const { return moveNumber_; }

   private:
    ::chess::Board board_;
    int moveNumber_;

    /**
     * @brief Find matching move from legal moves
     */
    std::optional<::chess::Move> findMove(const std::string& from, const std::string& to) const;

    /**
     * @brief Build StrikeData from a move
     */
    StrikeData buildStrikeData(const ::chess::Move& move) const;

    /**
     * @brief Get piece name as string
     */
    std::string getPieceName(::chess::PieceType type) const;
};