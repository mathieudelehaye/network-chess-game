#include "MoveParser.hpp"

#include "GameVisitor.hpp"
#include "Logger.hpp"
#include "SimpleChessGameLexer.h"
#include "SimpleChessGameParser.h"
#include "antlr4-runtime.h"

using namespace antlr4;
using namespace chess;

std::optional<ParsedMove> MoveParser::parse(const std::string& move) {
    auto& logger = Logger::instance();

    if (move.empty()) {
        logger.debug("Empty move string");
        return std::nullopt;
    }

    try {
        // Add newline to make it a valid game with one strike
        std::string move_as_game = move + "\n";

        // Create ANTLR parser stack
        ANTLRInputStream input(move_as_game);
        SimpleChessGameLexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        SimpleChessGameParser parser(&tokens);

        // Disable error messages to console
        parser.removeErrorListeners();

        // Parse as game (single strike + EOF)
        tree::ParseTree* tree = parser.game();

        // Check for syntax errors
        if (parser.getNumberOfSyntaxErrors() > 0) {
            logger.debug("Syntax errors in move: " + move);
            return std::nullopt;
        }

        // Visit the tree to extract move details
        GameVisitor visitor;
        visitor.visit(tree);

        // Extract moves from visitor
        auto moves = visitor.getMoves();

        if (moves.empty()) {
            logger.warning("Parser found no moves in: " + move);
            return std::nullopt;
        }

        // Get first (and should be only) move
        const auto& [from, to] = moves[0];

        return ParsedMove(from, to);

    } catch (const std::exception& e) {
        logger.error("Parser exception: " + std::string(e.what()));
        return std::nullopt;
    }
}

bool MoveParser::isValid(const std::string& move) {
    return parse(move).has_value();
}