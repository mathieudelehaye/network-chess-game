#pragma once

#include <any>
#include <string>
#include <utility>
#include <vector>

#include "SimpleChessGameBaseVisitor.h"
#include "SimpleChessGameParser.h"

// Custom visitor to handle parsed moves
class GameVisitor : public chess::SimpleChessGameBaseVisitor {
public:
    std::any visitStrike(chess::SimpleChessGameParser::StrikeContext* ctx) override;

    const std::vector<std::pair<std::string, std::string>>& getMoves() const {
        return moves_;
    }

private:
    // Parsed moves (from, to)
    std::vector<std::pair<std::string, std::string>> moves_;
};