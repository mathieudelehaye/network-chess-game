#include "GameVisitor.hpp"

std::any GameVisitor::visitStrike(chess::SimpleChessGameParser::StrikeContext* ctx) {
    // Extract the two coordinates (from and to)
    if (ctx->COORD().size() == 2) {
        std::string from = ctx->COORD(0)->getText();
        std::string to = ctx->COORD(1)->getText();
        moves_.push_back(std::make_pair(from, to));
    }

    return std::any();
}