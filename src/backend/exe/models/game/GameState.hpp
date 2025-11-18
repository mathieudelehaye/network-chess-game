#pragma once

enum class GameState {
    WaitingForStart,  // Game not started yet
    InProgress,       // Game is active
    Check,            // King in check
    Checkmate,        // Game over due to checkmate
    Stalemate,        // Game over due to stalemate
    Draw,
};

enum class Player {
    White,
    Black,
};