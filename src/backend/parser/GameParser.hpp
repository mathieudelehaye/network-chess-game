#pragma once

#include <optional>
#include <string>
#include <vector>

// Generic move structure that all parsers return
struct ParsedMove {
    std::string notation;  // The original notation (e.g., "e2-e4" or "Nf3")
    std::string from;      // Source square (if applicable, e.g., "e2")
    std::string to;        // Destination square (e.g., "e4")
    bool is_san;           // true if SAN notation, false if simple notation
};

// Parser interface - Strategy pattern
class IGameParser {
public:
    virtual ~IGameParser() = default;
    
    // Parse a full game file
    virtual std::optional<std::vector<ParsedMove>> parseGame(const std::string& game_data) = 0;
    
    // Parse a single move
    virtual std::optional<ParsedMove> parseMove(const std::string& move_str) = 0;
    
    // Get parser type name
    virtual std::string getParserType() const = 0;
};