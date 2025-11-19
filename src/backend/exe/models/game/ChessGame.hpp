#pragma once

#include <chess.hpp>
#include <optional>
#include <string>

#include "GameStatus.hpp"
#include "StrikeData.hpp"

class ChessGame {
public:
    ChessGame();
    ~ChessGame() = default;

    std::optional<StrikeData> applyMove(const std::string& from, const std::string& to);
    
    GameStatus getStatus() const;  // ← Changed return type
    
    chess::Color getCurrentPlayer() const;
    
    std::string getFEN() const;
    std::string getBoardASCII() const;
    bool isLegalMove(const std::string& from, const std::string& to) const;
    void reset();
    int getMoveNumber() const { return moveNumber_; }

private:
    chess::Board board_;
    int moveNumber_;

    std::optional<chess::Move> findMove(const std::string& from, const std::string& to) const;
    StrikeData buildStrikeData(const chess::Move& move) const;
    std::string getPieceName(chess::PieceType type) const;
};