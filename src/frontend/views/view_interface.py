"""View interface definition for game display.

Defines abstract interface for console and GUI view implementations.
"""

from abc import ABC, abstractmethod


class IView(ABC):
    """Minimal interface for game display methods.

    Defines methods that differ between Console and GUI modes.
    Implementations provide welcome display, board rendering,
    user input handling, and cleanup.
    """

    @abstractmethod
    def display_welcome(self) -> None:
        """Display welcome message."""
        pass

    @abstractmethod
    def display_board(self, board: str) -> None:
        """Display the chess board.

        Args:
            board: ASCII board string or data for GUI rendering
        """
        pass

    @abstractmethod
    def display_game_over(self, result: str) -> None:
        """Display game over screen.

        Args:
            result: Game result string
        """
        pass

    @abstractmethod
    def wait_for_input(self, info: dict) -> tuple[str, ...]:
        """Wait for user input.

        Args:
            info: Dictionary with context and model states

        Returns:
            tuple: User command and parameters
        """
        pass

    @abstractmethod
    def cleanup(self) -> None:
        """Clean up resources (close windows, etc.)."""
        pass
