#pragma once

#include <memory>
#include <string>

#include "ChessGame.hpp"

class GameController {
   public:
    GameController();
    ~GameController() = default;

    /**
     * @brief Handle a move request using parsed coordinates
     * @param from Source square (e.g., "e2")
     * @param to Destination square (e.g., "e4")
     * @return JSON response string
     */
    std::string handleMove(const std::string& from, const std::string& to);

    /**
     * @brief Start a new game
     */
    void startNewGame();

    /**
     * @brief Get current game state
     */
    const ChessGame& getGame() const { return *game_; }

   private:
    std::unique_ptr<ChessGame> game_;
};