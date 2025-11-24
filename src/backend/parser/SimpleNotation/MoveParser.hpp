#pragma once

#include <optional>
#include <string>
#include <vector>

/**
 * @brief Parsed move data
 */
struct SimpleNotationParsedMove {
    std::string from;  // e.g., "a2"
    std::string to;    // e.g., "a4"

    SimpleNotationParsedMove(const std::string& f, const std::string& t) : from(f), to(t) {}
};

/**
 * @brief Parses and validates chess moves using parser
 */
class MoveParser {
   public:
    MoveParser() = default;
    ~MoveParser() = default;

    /**
     * @brief Parse and validate a move string
     * @param move The move string (e.g., "a2-a4")
     * @return ParsedMove if valid, std::nullopt if invalid
     */
    std::optional<SimpleNotationParsedMove> parse(const std::string& move);

    /**
     * @brief Parse entire game file (for file upload mode)
     * @param move The file content
     * @return The parsed moves
     */
    std::vector<SimpleNotationParsedMove> parseGame(const std::string& game_content);
};