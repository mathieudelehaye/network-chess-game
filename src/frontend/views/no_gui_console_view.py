"""
View layer for displaying game information to the user, which are only used when
the ViewMode.NOGUI mode is selected. Some other console functions might be
shared between the ViewMode.NOGUI and ViewMode.GUI modes, and therefore will
be implemented by SharedConsoleView class instead.
"""

from utils.logger import Logger
from views.view_interface import IView


class NoGUIConsoleView(IView):
    """
    Console text-based view.
    Also contains all shared menu/input methods.
    """

    def __init__(self):
        self.logger_ = Logger()

    def display_welcome(self) -> None:
        """Display welcome message"""
        print("\n" + "=" * 60)
        print(" " * 15 + "CHESS GAME CLIENT")
        print("=" * 60 + "\n")

    def display_board(self, board: str) -> None:
        """Display chess board"""
        print("\n" + "=" * 50)
        print("Current Board:")
        print("=" * 50)
        print(board)
        print("=" * 50 + "\n")

    def display_game_over(self, result: str) -> None:
        """Display game over message"""
        print("\n" + "=" * 60)
        print(f"GAME OVER: {result.upper()}")
        print("=" * 60 + "\n")

    def wait_for_input(self, info: dict) -> tuple[str, ...]:
        """Get move from user (e.g., 'e2-e4').

        Special commands are also allowed:

        - :r => restart (only in single player mode)
        - :f <file name> => upload game file (only in single player)
        - :d => display board
        - :q => quit the game

        Args:
            info: Dictionary with game info

        Returns:
            tuple: Command type and parameters
        """

        single_player = info.get("player_number", 1) == 1
        gui_mode = info.get("view_mode", False)

        command = input("Enter move, e.g. e2-e4 (or special command): ").strip()

        # Handle empty input
        if not command:
            return (None, None)

        # Handle special commands
        if command[0] == ":":
            if len(command) < 2:
                # Invalid input command
                return (None, None)

            if single_player:
                # Resetting the game is only possible in single-player mode
                if command[1] == "r":
                    return ("r", None)

                # Uploading a game file is only possible in non-gui mode
                if not gui_mode and command[1] == "f":
                    pos = command[2:].find(" ")
                    if pos == -1:
                        # Invalid command usage
                        return (None, None)
                    file_path = command[2 + pos :].strip()
                    return ("f", file_path)

            if command[1] == "d":
                return ("d", None)

            if command[1] == "q":
                return ("q", None)

        # Handle regular move
        return ("m", command)

    def cleanup(self) -> None:
        """Clean up view resources"""
        pass
