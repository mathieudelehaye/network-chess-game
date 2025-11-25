#pragma once

#include <chess.hpp>
#include <optional>
#include <string>

#include "ParserFactory.hpp"
#include "StrikeData.hpp"

/**
 * @brief Core chess game model
 *
 * This class is a thin wrapper for the `chess.hpp` library
 * (https://github.com/Disservin/chess-library) which manages some internal
 * chess board states.
 */
class ChessGame {
   public:
    ChessGame();

    /**
     * @brief Apply a move and return strike data
     * @return StrikeData if move is valid, nullopt otherwise
     */
    std::optional<StrikeData> applyMove(const ParsedMove& move);

    /**
     * @brief Get current player color
     */
    chess::Color getCurrentPlayer() const;

    /**
     * @brief Get FEN notation of current position
     */
    std::string getFEN() const;

    /**
     * @brief Get ASCII representation of board
     */
    std::string getBoardFormatted() const;

    /**
     * @brief Reset to starting position
     */
    void reset();

   private:
    // Internal state queries
    bool isGameOver() const;
    bool inCheck() const;
    bool isCheckmate() const;
    bool isStalemate() const;

    // Helper methods
    std::optional<chess::Move> findMove(const std::string& from, const std::string& to) const;
    std::optional<chess::Move> findMoveFromSan(const std::string& san_move) const;

    void fillStrikeDataBeforeMove(StrikeData& data, const chess::Move& move) const;
    void fillStrikeDataAfterMove(StrikeData& data, const chess::Move& move) const;
    std::string getPieceName(chess::PieceType type) const;

    chess::Board board_;
    int moveNumber_;
};