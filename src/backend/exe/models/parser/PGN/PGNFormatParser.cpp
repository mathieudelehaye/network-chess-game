#include "PGNFormatParser.hpp"

#include <antlr4-runtime.h>

#include "PGNLexer.h"
#include "PGNParser.h"

using namespace antlr4;

PGNFormatParser::PGNFormatParser() : logger_(Logger::instance()) {
    logger_.debug("PGNParser initialised");
}

std::optional<std::vector<ParsedMove>> PGNFormatParser::parseGame(const std::string& game_data) {
    try {
        logger_.debug("Parsing PGN game for moves");
        
        ANTLRInputStream input(game_data);
        chess::PGNLexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        chess::PGNParser parser(&tokens);
        
        // Disable error messages to console
        parser.removeErrorListeners();
        
        auto* tree = parser.parse();
        
        PGNVisitor visitor;
        visitor.visit(tree);
        
        auto game_data = visitor.getGameData();
        if (!game_data) {
            logger_.warning("No game data extracted from PGN");
            return std::nullopt;
        }
        
        // Extract just the SAN strings
        std::vector<ParsedMove> san_moves;
        san_moves.reserve(game_data->moves.size());
        
        for (const auto& move : game_data->moves) {
            san_moves.push_back(convertToGenericMove(move));
            logger_.debug("Extracted move: " + move.san);
        }
        
        logger_.info("Parsed " + std::to_string(san_moves.size()) + " moves from PGN");
        return san_moves;
        
    } catch (const std::exception& e) {
        logger_.error("Error parsing PGN game: " + std::string(e.what()));
        return std::nullopt;
    }
}

std::optional<ParsedMove> PGNFormatParser::parseMove(const std::string& move_str) {
    try {
        logger_.debug("Parsing single SAN move: " + move_str);
        
        ANTLRInputStream input(move_str);
        chess::PGNLexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        chess::PGNParser parser(&tokens);
        
        parser.removeErrorListeners();
        
        auto* tree = parser.san_move(); 
        
        PGNVisitor visitor;
        visitor.visit(tree);
        
        auto move_data = visitor.getSingleMove();
        if (!move_data) {
            logger_.warning("Failed to parse move: " + move_str);
            return std::nullopt;
        }
        
        logger_.debug("Successfully parsed move: " + move_data->san);
        return convertToGenericMove(*move_data);
        
    } catch (const std::exception& e) {
        logger_.error("Error parsing SAN move: " + std::string(e.what()));
        return std::nullopt;
    }
}

ParsedMove PGNFormatParser::convertToGenericMove(const PGNMove& move) const {
    ParsedMove pm;
    pm.notation = move.san;
    pm.from = "";
    pm.to = "";
    pm.is_san = true;
    return pm;
}