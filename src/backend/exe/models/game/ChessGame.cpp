#include "ChessGame.hpp"

#include <sstream>
#include "Logger.hpp"

ChessGame::ChessGame() : moveNumber_(1) {
    board_.setFen(chess::constants::STARTPOS);
}

std::optional<StrikeData> ChessGame::applyMove(const std::string& from, const std::string& to) {
    auto move = findMove(from, to);
    if (!move) {
        return std::nullopt;
    }

    StrikeData data;

    fillStrikeDataBeforeMove(data, to);

    board_.makeMove(*move);

    fillStrikeDataAfterMove(data, *move);

    return data;
}

chess::Color ChessGame::getCurrentPlayer() const {
    return board_.sideToMove();
}

std::string ChessGame::getFEN() const {
    return board_.getFen();
}

std::string ChessGame::getBoardASCII() const {
    std::ostringstream oss;
    oss << board_;
    return oss.str();
}

bool ChessGame::isLegalMove(const std::string& from, const std::string& to) const {
    return findMove(from, to).has_value();
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

    for (const auto& move : moves) {
        if (move.from() == from_sq && move.to() == to_sq) {
            return move;
        }
    }

    return std::nullopt;
}

// TODO: consider returning a variable copy rather than passing the variable by
// reference 
void ChessGame::fillStrikeDataBeforeMove(
    StrikeData& data,
    const std::string& to
) const {
    // Check what's on the destination square
    auto destination = chess::Square(to);
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
void ChessGame::fillStrikeDataAfterMove(
    StrikeData& data,
    const chess::Move& move
) const {
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