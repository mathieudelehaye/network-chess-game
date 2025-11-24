#include "ParserFactory.hpp"

#include <algorithm>

#include "PGNFormatParser.hpp"
#include "PGNParser.h"
#include "SimpleNotationParser.hpp"
#include "Logger.hpp"

std::unique_ptr<IGameParser> ParserFactory::createParser(ParserType type) {
    auto& logger = Logger::instance();
    
    switch (type) {
        case ParserType::PGN:
        logger.info("Creating PGN parser");
        return std::make_unique<PGNFormatParser>();
        
        default:
        case ParserType::SIMPLE_NOTATION:
            logger.info("Creating SimpleNotation parser");
            return std::make_unique<SimpleNotationParser>();
    }
}

std::unique_ptr<IGameParser> ParserFactory::createParser(const std::string& type_str) {
    return createParser(parseParserType(type_str));
}

ParserType ParserFactory::parseParserType(const std::string& type_str) {
    // Convert to lowercase for case-insensitive comparison
    std::string lower = type_str;
    std::transform(lower.begin(), lower.end(), lower.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    
    auto& logger = Logger::instance();
    
    if (lower == "pgn") {
        logger.debug("Parsed parser type: PGN");
        return ParserType::PGN;
    } else if (lower == "simple" || lower == "simple_notation" || lower.empty()) {
        logger.debug("Parsed parser type: SimpleNotation");
        return ParserType::SIMPLE_NOTATION;
    }
    
    logger.warning("Unknown parser type: '" + type_str + "', defaulting to SimpleNotation");
    return ParserType::SIMPLE_NOTATION;
}