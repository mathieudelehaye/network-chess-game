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
            msg_type = msg.get("type")

            if msg_type == "error":
                self._handle_error(msg)
            elif msg_type == "board_display":
                self._handle_board_display(msg)
            elif msg_type == "move_response":
                self._handle_move_response(msg)
            elif msg_type == "game_response":
                self._handle_game_response(msg)
            elif msg_type == "upload_progress":
                self._handle_upload_progress(msg)
            else:
                self._logger.warning(f"Unknown message type: {msg_type}")

        except Exception as e:
            self._logger.error(f"Error routing message: {e}")

    def _handle_upload_progress(self, msg: dict) -> None:
        """Handle upload progress acknowledgment."""
        filename = msg.get("filename", "unknown")
        percent = msg.get("percent", 0)
        chunk = msg.get("chunk_received", 0)
        total = msg.get("chunks_total", 0)
        
        self._logger.debug(f"Server received: {filename} {percent}% ({chunk}/{total})")

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
        """Handle board display response."""
        data = msg.get("data", {})
        board_ascii = data.get("board", "")
        self._view.display_board(board_ascii)

    def _handle_move_response(self, msg: dict) -> None:
        """Handle validated move response."""
        data = msg.get("data", {})
        
        # Use MODEL to transform data
        move_description = self._model.build_move_description(data)
        
        # Use VIEW to display
        self._view.display_move(move_description)

        # Check for special game states
        if data.get("is_check"):
            self._view.display_info("Check!")
        
        if data.get("is_checkmate"):
            self._view.display_info("Checkmate!")
        
        if data.get("is_stalemate"):
            self._view.display_info("Stalemate!")

    def _handle_game_response(self, msg: dict) -> None:
        """
        Handle game file processing response.
        Displays all moves sequentially.
        """
        filename = msg.get("filename", "unknown")
        total_moves = msg.get("total_moves", 0)
        moves = msg.get("moves", [])

        self._logger.info(f"Game: {filename} ({total_moves} moves)")
        self._view.display_info(f"\n{'='*50}")
        self._view.display_info(f"Playing game from: {filename}")
        self._view.display_info(f"{'='*50}\n")

        successful_moves = 0
        errors = 0

        # Display each move
        for idx, move in enumerate(moves, start=1):
            if move.get("type") == "error":
                error_msg = move.get('error', 'Unknown error')
                line_num = move.get('data', {}).get('line', idx)
                self._view.display_error(f"Line {line_num}: {error_msg}")
                errors += 1
                # Don't break - continue showing other moves
            elif move.get("type") == "move_response":
                data = move.get("data", {})
                description = self._model.build_move_description(data)
                self._view.display_move(description)
                successful_moves += 1

                # Show special states
                if data.get("is_check"):
                    self._view.display_info("  → Check!")
                if data.get("is_checkmate"):
                    self._view.display_info("  → Checkmate!")
                    break  # Game over on checkmate

        self._view.display_info(f"\n{'='*50}")
        self._view.display_info(f"Game completed: {successful_moves} moves, {errors} errors")
        self._view.display_info(f"{'='*50}\n")

    def _handle_game_over(self, msg: dict) -> None:
        """
        Handle game over message.

        Args:
            msg: Game over message
        """
        result = msg.get("result", "unknown")
        self._view.display_game_over(result)
