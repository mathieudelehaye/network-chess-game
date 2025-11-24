#include "SimpleNotationParser.hpp"

SimpleNotationParser::SimpleNotationParser() : logger_(Logger::instance()) {
    logger_.debug("SimpleNotationParser initialised");
}

std::optional<std::vector<ParsedMove>> SimpleNotationParser::parseGame(const std::string& game_data) {
    logger_.debug("Parsing game with SimpleNotation parser");
    
    // Use the low-level MoveParser to do ANTLR parsing
    auto simple_moves = move_parser_.parseGame(game_data);
    
    if (simple_moves.empty()) {
        logger_.warning("No moves parsed from game data");
        return std::nullopt;
    }
    
    // Convert to generic ParsedMove format
    std::vector<ParsedMove> result;
    result.reserve(simple_moves.size());
    
    for (const auto& simple_move : simple_moves) {
        result.push_back(convertToGenericMove(simple_move));
        logger_.debug("Parsed move: " + simple_move.from + " -> " + simple_move.to);
    }
    
    logger_.info("Parsed " + std::to_string(result.size()) + " moves in simple notation");
    return result;
}

std::optional<ParsedMove> SimpleNotationParser::parseMove(const std::string& move_str) {
    logger_.debug("Parsing single move: " + move_str);
    
    // Use the low-level MoveParser
    auto simple_move = move_parser_.parse(move_str);
    
    if (!simple_move) {
        logger_.warning("Failed to parse move: " + move_str);
        return std::nullopt;
    }
    
    // Convert to generic format
    ParsedMove result = convertToGenericMove(*simple_move);
    
    logger_.debug("Successfully parsed: " + result.notation);
    return result;
}

ParsedMove SimpleNotationParser::convertToGenericMove(const SimpleNotationParsedMove& move) const {
    ParsedMove pm;
    pm.notation = move.from + "-" + move.to;
    pm.from = move.from;
    pm.to = move.to;
    pm.is_san = false;
    return pm;
}