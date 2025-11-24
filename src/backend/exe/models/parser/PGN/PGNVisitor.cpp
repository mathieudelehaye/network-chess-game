#include "PGNVisitor.hpp"

PGNVisitor::PGNVisitor() : logger_(Logger::instance()) {
    logger_.debug("PGNVisitor initialised");
}

std::any PGNVisitor::visitFullGame(chess::PGNParser::FullGameContext* ctx) {
    logger_.debug("Visiting full PGN game");
    
    game_data_ = PGNGameData();
    
    if (ctx->pgn_game()) {
        visitPgn_game(ctx->pgn_game());
    }
    
    return game_data_;
}

std::any PGNVisitor::visitSingleMove(chess::PGNParser::SingleMoveContext* ctx) {
    logger_.debug("Visiting single move");
    
    if (ctx->san_move()) {
        single_move_ = std::any_cast<PGNMove>(visit(ctx->san_move()));
    }
    
    return single_move_;
}

std::any PGNVisitor::visitPgn_game(chess::PGNParser::Pgn_gameContext* ctx) {
    // Visit tag section (metadata)
    if (ctx->tag_section()) {
        for (auto* tag : ctx->tag_section()->tag_pair()) {
            visitTag_pair(tag);
        }
    }
    
    // Visit movetext section (the actual moves)
    if (ctx->movetext_section()) {
        auto* element_seq = ctx->movetext_section()->element_sequence();
        if (element_seq) {
            for (auto* element : element_seq->element()) {
                visitElement(element);
            }
        }
    }
    
    return game_data_;
}

std::any PGNVisitor::visitTag_pair(chess::PGNParser::Tag_pairContext* ctx) {
    std::string name = ctx->tag_name()->getText();
    std::string value = ctx->tag_value()->getText();
    
    // Remove quotes from value
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        value = value.substr(1, value.size() - 2);
    }
    
    logger_.debug("Tag: " + name + " = " + value);
    
    if (!game_data_) {
        game_data_ = PGNGameData();
    }
    
    // Store metadata
    if (name == "Event") game_data_->event = value;
    else if (name == "Site") game_data_->site = value;
    else if (name == "Date") game_data_->date = value;
    else if (name == "Round") game_data_->round = value;
    else if (name == "White") game_data_->white = value;
    else if (name == "Black") game_data_->black = value;
    else if (name == "Result") game_data_->result = value;
    
    return nullptr;
}

std::any PGNVisitor::visitElement(chess::PGNParser::ElementContext* ctx) {
    // Only process actual moves, skip move numbers and comments
    if (ctx->san_move()) {
        PGNMove move = std::any_cast<PGNMove>(visit(ctx->san_move()));
        if (game_data_) {
            game_data_->moves.push_back(move);
            logger_.debug("Added move: " + move.san);
        }
    }
    return nullptr;
}

std::any PGNVisitor::visitCastleKingside(chess::PGNParser::CastleKingsideContext* ctx) {
    PGNMove move;
    move.san = ctx->getText();  // "O-O", "O-O+", or "O-O#"

    // CRITICAL: Store move the member variable so getSingleMove() can find it
    this->single_move_ = move; 
    
    logger_.debug("Parsed kingside castle: " + move.san);
    return move;
}

std::any PGNVisitor::visitCastleQueenside(chess::PGNParser::CastleQueensideContext* ctx) {
    PGNMove move;
    move.san = ctx->getText();  // "O-O-O", "O-O-O+", or "O-O-O#"

    // CRITICAL: Store move the member variable so getSingleMove() can find it
    this->single_move_ = move; 
    
    logger_.debug("Parsed queenside castle: " + move.san);
    return move;
}

std::any PGNVisitor::visitPawnMove(chess::PGNParser::PawnMoveContext* ctx) {
    PGNMove move;
    move.san = ctx->getText();  // e.g., "e4", "exd5", "e8=Q#"

    // CRITICAL: Store move the member variable so getSingleMove() can find it
    this->single_move_ = move; 
    
    logger_.debug("Parsed pawn move: " + move.san);
    return move;
}

std::any PGNVisitor::visitPieceMove(chess::PGNParser::PieceMoveContext* ctx) {
    PGNMove move;
    move.san = ctx->getText();  // e.g., "Nf3", "Qxh7+", "Rad1"
    
    // CRITICAL: Store move the member variable so getSingleMove() can find it
    this->single_move_ = move; 

    logger_.debug("Parsed piece move: " + move.san);
    return move;
}