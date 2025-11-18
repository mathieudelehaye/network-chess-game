#include "GameVisitor.hpp"

std::any GameVisitor::visitStrike(chess::SimpleChessGameParser::StrikeContext *ctx) {
    std::string from = ctx->COORD(0)->getText();
    std::string to = ctx->COORD(1)->getText();
    moves.push_back(make_pair(from, to));
    return {};
}