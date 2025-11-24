#pragma once

#include <optional>
#include <string>
#include <vector>

#include "GameParser.hpp"
#include "Logger.hpp"
#include "PGNVisitor.hpp"

class PGNFormatParser : public IGameParser {
public:
    PGNFormatParser();
    
    // Parse a full PGN game file and return list of SAN moves
    std::optional<std::vector<ParsedMove>> parseGame(const std::string& game_data) override;
    
    // Parse and validate a single SAN move (e.g., "Nf3", "exd5", "O-O")
    std::optional<ParsedMove> parseMove(const std::string& move_str) override;
    
    std::string getParserType() const override { return "PGN"; }

private:
    // Helper to convert PGNMove to ParsedMove
    ParsedMove convertToGenericMove(const PGNMove& move) const;

    Logger& logger_;
};