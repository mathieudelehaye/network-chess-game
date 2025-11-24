#pragma once

#include <string>

/**
 * @brief Validated strike information to send to client
 */
struct StrikeData {
    std::string piece;      // "pawn", "knight", "bishop", "rook", "queen", "king"
    std::string color;      // "white", "black"
    std::string case_src;   // e.g., "e2"
    std::string case_dest;  // e.g., "e4"
    int strike_number;      // Move number

    // Optional fields
    bool is_capture = false;
    std::string captured_piece;  // If is_capture, what piece was taken
    std::string captured_color;  // Color of captured piece

    bool is_castling = false;
    std::string castling_type;  // "big" or "little"

    bool is_check = false;
    bool is_checkmate = false;
    bool is_stalemate = false;
};