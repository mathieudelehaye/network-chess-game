#pragma once

#include "GameParser.hpp"
#include "MoveParser.hpp"
#include "Logger.hpp"

/**
 * @brief Thin-wrapper for MoveParser implemeting the Strategy pattern for the
 * simple chess notation (e2-e4 format)
 *
 * This parser handles moves in the format:
 * - "e2-e4"
 * - "e2 e4"
 * - "e2->e4"
 *
 * Implements the IGameParser interface for use with the factory pattern
 */
class SimpleNotationParser : public IGameParser {
public:
    SimpleNotationParser();
    ~SimpleNotationParser() override = default;
    
    /**
     * @brief Parse a full game file with simple notation moves
     * @param game_data The raw game file content
     * @return List of parsed moves, or nullopt if parsing failed
     */
    std::optional<std::vector<ParsedMove>> parseGame(const std::string& game_data) override;
    
    /**
     * @brief Parse a single move in simple notation
     * @param move_str The move string (e.g., "e2-e4")
     * @return Parsed move, or nullopt if parsing failed
     */
    std::optional<ParsedMove> parseMove(const std::string& move_str) override;
    
    /**
     * @brief Get the parser type name
     * @return "SimpleNotation"
     */
    std::string getParserType() const override { return "SimpleNotation"; }

private:
    Logger& logger_;
    MoveParser move_parser_;  // Delegates to low-level ANTLR parser
    
    // Helper to convert SimpleMove to ParsedMove
    ParsedMove convertToGenericMove(const SimpleNotationParsedMove& move) const;
};