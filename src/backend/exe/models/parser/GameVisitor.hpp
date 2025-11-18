#pragma once

#include <vector>

#include "SimpleChessGameBaseVisitor.h"
#include "SimpleChessGameLexer.h"
#include "SimpleChessGameParser.h"
#include "antlr4-runtime.h"

// Custom visitor to handle parsed moves
class GameVisitor : public chess::SimpleChessGameBaseVisitor {
    // Parsed moves
    std::vector<std::pair<std::string, std::string>> moves;

   public:
    std::any visitStrike(chess::SimpleChessGameParser::StrikeContext *ctx) override;

    const std::vector<std::pair<std::string, std::string>> getMoves() const { return moves; };
};