#pragma once

#include <optional>
#include <string>
#include <vector>

#include "PGNBaseVisitor.h"
#include "PGNParser.h"
#include "Logger.hpp"


// Simplified move structure - just stores the SAN string
struct PGNMove {
    std::string san;  // The raw SAN notation to pass to chess library
};

// Game metadata structure
struct PGNGameData {
    std::string event;
    std::string site;
    std::string date;
    std::string round;
    std::string white;
    std::string black;
    std::string result;
    std::vector<PGNMove> moves;  // List of SAN moves
};

class PGNVisitor : public chess::PGNBaseVisitor {
public:
    PGNVisitor();
    
    // Visit full game
    std::any visitFullGame(chess::PGNParser::FullGameContext* ctx) override;
    
    // Visit single move
    std::any visitSingleMove(chess::PGNParser::SingleMoveContext* ctx) override;
    
    // Visit game components
    std::any visitPgn_game(chess::PGNParser::Pgn_gameContext* ctx) override;
    std::any visitTag_pair(chess::PGNParser::Tag_pairContext* ctx) override;
    std::any visitElement(chess::PGNParser::ElementContext* ctx) override;
    
    // Visit move types - all just extract the SAN text
    std::any visitCastleKingside(chess::PGNParser::CastleKingsideContext* ctx) override;
    std::any visitCastleQueenside(chess::PGNParser::CastleQueensideContext* ctx) override;
    std::any visitPawnMove(chess::PGNParser::PawnMoveContext* ctx) override;
    std::any visitPieceMove(chess::PGNParser::PieceMoveContext* ctx) override;
    
    // Get results
    std::optional<PGNGameData> getGameData() const { return game_data_; }
    std::optional<PGNMove> getSingleMove() const { return single_move_; }

private:
    Logger& logger_;
    std::optional<PGNGameData> game_data_;
    std::optional<PGNMove> single_move_;
};