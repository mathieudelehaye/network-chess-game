#pragma once

#include <memory>
#include <string>

#include "GameParser.hpp"

/**
 * @brief Available parser types
 */
enum class ParserType {
    SIMPLE_NOTATION,  // e2-e4 format
    PGN               // Standard Algebraic Notation (Nf3, O-O, etc.)
};

/**
 * @brief Factory for creating chess notation parsers
 * 
 * Uses Factory pattern to create the appropriate parser based on
 * user configuration or command-line arguments.
 */
class ParserFactory {
public:
    /**
     * @brief Create a parser by type enum
     * @param type The parser type to create
     * @return Unique pointer to the created parser
     */
    static std::unique_ptr<IGameParser> createParser(ParserType type);
    
    /**
     * @brief Create a parser by type string (from command line)
     * @param type_str String representation ("simple", "pgn")
     * @return Unique pointer to the created parser
     */
    static std::unique_ptr<IGameParser> createParser(const std::string& type_str);
    
    /**
     * @brief Parse parser type from string
     * @param type_str String to parse
     * @return Corresponding ParserType enum value
     */
    static ParserType parseParserType(const std::string& type_str);
};