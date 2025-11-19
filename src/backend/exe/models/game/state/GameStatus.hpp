#pragma once

/**
 * @brief Chess game status
 */
enum class GameStatus {
    IN_PROGRESS,
    CHECK,
    CHECKMATE,
    STALEMATE,
    DRAW
};