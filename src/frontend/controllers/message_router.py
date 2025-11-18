"""
Message router for client-side message handling.
Mirrors the server's MessageRouter architecture.
"""

from utils.logger import Logger
from views.console_view import ConsoleView
from models.game_model import GameModel


class MessageRouter:
    """
    Routes incoming server messages to appropriate handlers.
    """

    def __init__(self):
        self._logger = Logger()
        self._view = ConsoleView()
        self._model = GameModel()

    def route(self, msg: dict) -> None:
        """
        Route a message from the server to the appropriate handler.

        Args:
            msg: Parsed JSON message from server
        """
        try:
            # Handle error responses
            if "error" in msg:
                self._handle_error(msg)
                return

            # Handle board display response
            if "board" in msg:
                self._handle_board_display(msg)
                return

            # Handle validated move response
            if "strike_number" in msg and "piece" in msg:
                self._handle_move_response(msg)
                return

            # Handle game over
            if msg.get("game_over"):
                self._handle_game_over(msg)
                return

            # Unknown message type
            self._logger.warning(f"Unknown message type: {msg}")

        except Exception as e:
            self._logger.error(f"Error routing message: {e}")

    def _handle_error(self, msg: dict) -> None:
        """
        Handle error responses from server.

        Args:
            msg: Error message from server
        """
        error = msg.get("error", "Unknown error")
        self._view.display_error(error)

        if "expected" in msg:
            self._view.display_info(f"Expected format: {msg['expected']}")

    def _handle_board_display(self, msg: dict) -> None:
        """
        Handle board display response.

        Args:
            msg: Board display message
        """
        board_ascii = msg.get("board", "")
        self._view.display_board(board_ascii)

    def _handle_move_response(self, msg: dict) -> None:
        """
        Handle validated move response from server.

        Args:
            msg: Strike data from server
        """
        # Build human-readable move description from server data
        move_description = self._model.build_move_description(msg)
        
        # Use VIEW to display
        self._view.display_move(move_description)

        # Check for special game states
        if msg.get("is_check"):
            self._view.display_info("Check!")
        
        if msg.get("is_checkmate"):
            self._view.display_info("Checkmate!")
        
        if msg.get("is_stalemate"):
            self._view.display_info("Stalemate!")

    def _handle_game_over(self, msg: dict) -> None:
        """
        Handle game over message.

        Args:
            msg: Game over message
        """
        result = msg.get("result", "unknown")
        self._view.display_game_over(result)
