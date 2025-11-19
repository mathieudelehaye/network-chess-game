#pragma once

#include <memory>
#include <string>

#include "ChessGame.hpp"

class GameController {
   public:
    GameController();
    ~GameController() = default;

    /**
     * @brief Handle a board display request
     * @return JSON response string with board ASCII
     */
    std::string handleDisplayBoard();

   private:
    std::unique_ptr<ChessGame> game_;
};