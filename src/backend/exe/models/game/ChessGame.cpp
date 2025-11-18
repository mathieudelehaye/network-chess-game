#include "ChessGame.hpp"

#include <sstream>

#include "Logger.hpp"

ChessGame::ChessGame()
    : board_(::chess::Board())  // Standard starting position
      ,
      moveNumber_(1) {}

std::optional<::chess::Move> ChessGame::findMove(const std::string& from,
                                                 const std::string& to) const {
    try {
        // Convert your format "a2-a4" to UCI format "a2a4"
        std::string uciMove = from + to;

        // Generate ALL legal moves for current position
        ::chess::Movelist legalMoves;
        ::chess::movegen::legalmoves(legalMoves, board_);

        // Parse the requested move
        auto requestedMove = ::chess::uci::uciToMove(board_, uciMove);

        // Check if it's NO_MOVE (syntactically invalid)
        if (requestedMove == ::chess::Move::NO_MOVE) {
            return std::nullopt;
        }

        // Verify the move is actually in the legal moves list
        for (const auto& legalMove : legalMoves) {
            if (legalMove == requestedMove) {
                return legalMove;
            }
        }

        // Move is syntactically valid but chess-illegal
        Logger::instance().debug("Move not in legal moves: " + uciMove);
        return std::nullopt;

    } catch (...) {
        return std::nullopt;
    }
}

bool ChessGame::isLegalMove(const std::string& from, const std::string& to) const {
    return findMove(from, to).has_value();
}

std::string ChessGame::getPieceName(::chess::PieceType type) const {
    if (type == ::chess::PieceType::PAWN)
        return "pawn";
    if (type == ::chess::PieceType::KNIGHT)
        return "knight";
    if (type == ::chess::PieceType::BISHOP)
        return "bishop";
    if (type == ::chess::PieceType::ROOK)
        return "rook";
    if (type == ::chess::PieceType::QUEEN)
        return "queen";
    if (type == ::chess::PieceType::KING)
        return "king";
    return "piece";
}

StrikeData ChessGame::buildStrikeData(const ::chess::Move& move) const {
    StrikeData data;

    // Get piece at source square
    auto piece = board_.at(move.from());
    auto pieceType = piece.type();

    // Basic move info
    data.piece = getPieceName(pieceType);
    data.color = (board_.sideToMove() == ::chess::Color::WHITE) ? "white" : "black";

    // Convert squares to strings (e.g., "e2", "e4")
    std::string uciStr = ::chess::uci::moveToUci(move);
    data.case_src = uciStr.substr(0, 2);
    data.case_dest = uciStr.substr(2, 2);

    data.strike_number = moveNumber_;

    // Check for capture
    auto targetPiece = board_.at(move.to());
    if (targetPiece != ::chess::Piece::NONE) {
        data.is_capture = true;
        data.captured_piece = getPieceName(targetPiece.type());
        data.captured_color = (data.color == "white") ? "black" : "white";
    }

    // Check for castling using move typeOf
    if (move.typeOf() == ::chess::Move::CASTLING) {
        data.is_castling = true;
        // Kingside (short) castling: king moves to g-file
        data.castling_type = (move.to().file() == ::chess::File::FILE_G) ? "little" : "big";
    }

    return data;
}

std::optional<StrikeData> ChessGame::applyMove(const std::string& from, const std::string& to) {
    auto& logger = Logger::instance();

    auto move = findMove(from, to);
    if (!move) {
        logger.debug("Illegal move: " + from + "-" + to);
        return std::nullopt;
    }

    // Build strike data BEFORE applying move (board state changes after)
    StrikeData data = buildStrikeData(*move);

    // Apply move
    board_.makeMove(*move);

    // Check game state AFTER move is applied
    if (board_.isGameOver().second == ::chess::GameResult::LOSE) {
        data.is_checkmate = true;
    } else if (board_.isGameOver().second == ::chess::GameResult::DRAW) {
        data.is_stalemate = true;
    } else if (board_.inCheck()) {
        data.is_check = true;
    }

    // Increment move number (every 2 half-moves)
    if (board_.sideToMove() == ::chess::Color::WHITE) {
        moveNumber_++;
    }

    logger.info("Move applied: " + data.color + " " + data.piece + " from " + data.case_src +
                " to " + data.case_dest);

    return data;
}

GameState ChessGame::getState() const {
    if (board_.isGameOver().second == ::chess::GameResult::LOSE) {
        return GameState::Checkmate;
    }
    if (board_.isGameOver().second == ::chess::GameResult::DRAW) {
        return GameState::Draw;
    }
    if (board_.inCheck()) {
        return GameState::Check;
    }
    return GameState::InProgress;
}

Player ChessGame::getCurrentPlayer() const {
    return (board_.sideToMove() == ::chess::Color::WHITE) ? Player::White : Player::Black;
}

std::string ChessGame::getFEN() const {
    return board_.getFen();
}

void ChessGame::reset() {
    board_.setFen(::chess::constants::STARTPOS);
    moveNumber_ = 1;
}

std::string ChessGame::getBoardASCII() const {
    std::ostringstream board;

    // Header
    board << " a b c d e f g h\n";
    board << " ---------------------------------\n";

    // Rows (8 to 1, top to bottom)
    for (int rank = 7; rank >= 0; --rank) {
        board << (rank + 1) << " |";  // Row number

        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            auto piece = board_.at(::chess::Square(square));

            char pieceChar = ' ';
            if (piece != ::chess::Piece::NONE) {
                auto type = piece.type();
                auto color = piece.color();

                // Get base character
                if (type == ::chess::PieceType::PAWN)
                    pieceChar = 'p';
                else if (type == ::chess::PieceType::KNIGHT)
                    pieceChar = 'c';
                else if (type == ::chess::PieceType::BISHOP)
                    pieceChar = 'b';
                else if (type == ::chess::PieceType::ROOK)
                    pieceChar = 'r';
                else if (type == ::chess::PieceType::QUEEN)
                    pieceChar = 'q';
                else if (type == ::chess::PieceType::KING)
                    pieceChar = 'k';
                else
                    pieceChar = '?';

                // Uppercase for white, lowercase for black
                if (color == ::chess::Color::WHITE) {
                    pieceChar = std::toupper(pieceChar);
                }
            }

            board << " " << pieceChar << " |";
        }

        board << "\n ---------------------------------\n";
    }

    // Footer
    board << " a b c d e f g h\n";

    return board.str();
}