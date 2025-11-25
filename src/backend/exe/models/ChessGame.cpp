#include "ChessGame.hpp"

#include <sstream>

#include "Logger.hpp"

ChessGame::ChessGame() : moveNumber_(1) {
    board_.setFen(chess::constants::STARTPOS);
}

std::optional<StrikeData> ChessGame::applyMove(const ParsedMove& move) {
    auto chess_move = (move.is_san ? findMoveFromSan(move.notation) : findMove(move.from, move.to));

    if (!chess_move) {
        return std::nullopt;
    }

    if (chess_move->typeOf() == chess::Move::CASTLING) {
        auto& logger = Logger::instance();
        logger.debug("Castling move detected");
    }

    StrikeData data;

    fillStrikeDataBeforeMove(data, *chess_move);
    board_.makeMove(*chess_move);
    moveNumber_++;  // Need to manually increment move number
    fillStrikeDataAfterMove(data, *chess_move);

    return data;
}

chess::Color ChessGame::getCurrentPlayer() const {
    return board_.sideToMove();
}

std::string ChessGame::getFEN() const {
    return board_.getFen();
}

void ChessGame::reset() {
    board_.setFen(chess::constants::STARTPOS);
    moveNumber_ = 1;
}

bool ChessGame::isGameOver() const {
    auto [reason, result] = board_.isGameOver();
    return result == chess::GameResult::LOSE;
}

bool ChessGame::inCheck() const {
    return board_.inCheck();
}

bool ChessGame::isCheckmate() const {
    auto [reason, result] = board_.isGameOver();
    return result == chess::GameResult::LOSE;
}

bool ChessGame::isStalemate() const {
    auto [reason, result] = board_.isGameOver();
    return result == chess::GameResult::DRAW;
}

std::optional<chess::Move> ChessGame::findMove(const std::string& from,
                                               const std::string& to) const {
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board_);

    chess::Square from_sq(from);
    chess::Square to_sq(to);

    auto& logger = Logger::instance();

    if (logger.isLevelTrace()) {
        std::ostringstream traceStream;

        for (const auto& move : moves) {
            traceStream << "Move from: " << move.from() << ", to: " << move.to()
                        << ", isCastling: " << (move.typeOf() == chess::Move::CASTLING) << "\n";
        }

        logger.trace(traceStream.str());
    }

    for (const auto& move : moves) {
        if (move.from() == from_sq && move.to() == to_sq) {
            return move;
        }
    }

    return std::nullopt;
}

std::optional<chess::Move> ChessGame::findMoveFromSan(const std::string& san_move) const {
    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board_);

    for (const auto& move : moves) {
        // The library provides a utility to convert a Move object + Board State -> SAN string
        // We generate the SAN for every legal move and see if it matches the user input.
        std::string generatedSan = chess::uci::moveToSan(board_, move);

        // 1. Direct Match (Most accurate)
        if (generatedSan == san_move) {
            return move;
        }

        // 2. Loose Match (Optional but recommended for User Input)
        // Users often type "Nf3" even if the actual move is "Nf3+" (check) or "Nf3#" (mate).
        // This block handles that case by stripping symbols from the generated move.
        std::string cleanSan = generatedSan;
        if (!cleanSan.empty() && (cleanSan.back() == '+' || cleanSan.back() == '#')) {
            cleanSan.pop_back();
        }

        // Also strip input just in case user typed "Nf3+" but it wasn't a check
        std::string cleanInput = san_move;
        if (!cleanInput.empty() && (cleanInput.back() == '+' || cleanInput.back() == '#')) {
            cleanInput.pop_back();
        }

        if (cleanSan == cleanInput) {
            return move;
        }
    }

    return std::nullopt;
}

// TODO: consider returning a variable copy rather than passing the variable by
// reference
void ChessGame::fillStrikeDataBeforeMove(StrikeData& data, const chess::Move& move) const {
    // Check what's on the destination square
    auto destination = move.to();
    auto captured = board_.at(destination);

    data.is_capture = (captured != chess::Piece::NONE);

    if (data.is_capture) {
        data.captured_color = (moveNumber_ % 2 == 1) ? "black" : "white";
        data.captured_piece = getPieceName(captured.type());
    }

    data.strike_number = moveNumber_;
    data.color = (moveNumber_ % 2 == 1) ? "white" : "black";
}

// TODO: consider returning a variable copy rather than passing the variable by
// reference
void ChessGame::fillStrikeDataAfterMove(StrikeData& data, const chess::Move& move) const {
    data.case_src = std::string(move.from());
    data.case_dest = std::string(move.to());
    data.piece = getPieceName(board_.at(move.to()).type());

    data.is_check = inCheck();
    data.is_checkmate = isCheckmate();
    data.is_stalemate = isStalemate();

    if (move.typeOf() == chess::Move::CASTLING) {
        data.is_castling = true;

        // Determine castling type based on the destination square
        // King-side castling: king moves to g-file
        // Queen-side castling: king moves to c-file
        data.castling_type = (move.to().file() == chess::File::FILE_G) ? "little" : "big";
    }
}

std::string ChessGame::getPieceName(chess::PieceType type) const {
    if (type == chess::PieceType::PAWN)
        return "pawn";
    if (type == chess::PieceType::KNIGHT)
        return "knight";
    if (type == chess::PieceType::BISHOP)
        return "bishop";
    if (type == chess::PieceType::ROOK)
        return "rook";
    if (type == chess::PieceType::QUEEN)
        return "queen";
    if (type == chess::PieceType::KING)
        return "king";
    return "unknown";
}

std::string ChessGame::getBoardFormatted() const {
    std::ostringstream oss;
    oss << board_;
    std::string ascii_board = oss.str();

    // Extract only the board lines (first 8 lines)
    std::vector<std::string> lines;
    std::istringstream stream(ascii_board);
    std::string line;

    while (std::getline(stream, line) && lines.size() < 8) {
        lines.push_back(line);
    }

    if (lines.size() != 8) {
        return "Error: Invalid board format";
    }

    std::ostringstream result;

    // Top file labels
    result << "    a   b   c   d   e   f   g   h\n";
    result << " ---------------------------------\n";

    // Process each rank (8 to 1)
    for (int rank = 0; rank < 8; ++rank) {
        int rank_num = 8 - rank;

        // Parse pieces from the line
        std::vector<char> pieces;
        std::istringstream line_stream(lines[rank]);
        char piece;

        while (line_stream >> piece) {
            pieces.push_back(piece);
        }

        if (pieces.size() != 8) {
            return "Error: Invalid board row";
        }

        // Build the row
        result << rank_num << " |";

        for (char p : pieces) {
            // Convert knight 'n' to 'c' and empty '.' to space
            if (p == 'n')
                p = 'c';
            else if (p == 'N')
                p = 'C';
            else if (p == '.')
                p = ' ';

            result << " " << p << " |";
        }

        result << "\n ---------------------------------\n";
    }

    // Bottom file labels
    result << "    a   b   c   d   e   f   g   h\n";

    return result.str();
}