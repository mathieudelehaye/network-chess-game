from abc import ABC, abstractmethod


class IView(ABC):
    """
    Minimal interface for game display methods.
    Only methods that differ between Console and GUI modes.

    TODO: extend this interface to cover (most) of view functionalities, avoiding so the current "console - GUI" hybrid mode.
    """

    @abstractmethod
    def display_welcome(self) -> None:
        """Display welcome message"""
        pass

    @abstractmethod
    def display_board(self, board: str) -> None:
        """
        Display the chess board
        
        @param board_state: ASCII board string to be plotted as is or turned into a GUI viewport
        """
        pass

    @abstractmethod
    def display_game_over(self, result: str) -> None:
        """Display game over screen"""
        pass

    @abstractmethod
    def wait_for_input(self, info: dict) -> tuple[str, ...]:
        """
        Wait for user input
        
        @param info: dictionary with context and model states
        @return: tuple with user command and parameters
        """
        pass

    @abstractmethod
    def cleanup(self) -> None:
        """Clean up resources (close windows, etc.)"""
        pass