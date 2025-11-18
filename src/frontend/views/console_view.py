"""
View layer for displaying game information to the user.
"""

from utils.logger import Logger


class ConsoleView:
    """
    Handles all user-facing output.
    Separates presentation logic from business logic.
    """

    def __init__(self):
        self._logger = Logger()

    def display_move(self, description: str) -> None:
        """
        Display a move description.

        Args:
            description: Human-readable move description
        """
        self._logger.info(description)

    def display_board(self, board_ascii: str) -> None:
        """
        Display the chess board.

        Args:
            board_ascii: ASCII representation of the board
        """
        print("\n" + board_ascii)

    def display_error(self, error: str) -> None:
        """
        Display an error message.

        Args:
            error: Error message
        """
        self._logger.error(f"Server error: {error}")

    def display_info(self, info: str) -> None:
        """
        Display informational message.

        Args:
            info: Information to display
        """
        self._logger.info(info)

    def display_game_over(self, result: str) -> None:
        """
        Display game over message.

        Args:
            result: Game result (checkmate, stalemate, draw)
        """
        self._logger.info(f"Game Over: {result}")

    def display_prompt(self, prompt: str) -> None:
        """
        Display a user prompt.

        Args:
            prompt: Prompt text
        """
        print(f"\n{prompt}", end="")