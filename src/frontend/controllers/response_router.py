"""Response router for handling server messages.

Routes incoming JSON messages to appropriate handlers based on type.
"""

import json
from utils.logger import Logger
from views.view_factory import ViewMode


class ResponseRouter:
    """Routes incoming server messages to appropriate handlers.

    Parses JSON messages and dispatches to type-specific handler methods.
    Coordinates updates to context, model, and views based on server responses.
    """

    def __init__(
        self,
        game_controller,
        view_mode: ViewMode,
    ):
        """Initialize message router.

        Args:
            game_controller: GameController reference for state access
            view_mode: Display mode (GUI or console)
        """

        self.logger_ = Logger()
        self.game_controller = game_controller
        self.view_mode = view_mode

        # Get references from game controller (singleton instances)
        self.context = game_controller.context
        self.model = game_controller.model

        # Get view references from game controller
        self.game_view = game_controller.game_view
        self.console_view = game_controller.console_view

    def route(self, message: str):
        """Route incoming server message to appropriate handler.

        Args:
            message: JSON message string from server
        """

        try:
            response = json.loads(message)

        except json.JSONDecodeError as e:
            self.logger_.error(f"Invalid JSON from server: {e}")
            self.logger_.debug(f"Raw data: {message}")
            return

        msg_type = response.get("type", "unknown")
        self.logger_.debug(f"Routing message type: {msg_type}")

        # Route to handler
        handlers = {
            "session_created": self._handle_session_created,
            "join_success": self._handle_join_success,
            "player_joined": self._handle_player_joined,
            "game_ready": self._handle_game_ready,
            "game_started": self._handle_game_started,
            "move_result": self._handle_move_result,
            "board_display": self._handle_board_display,
            "game_over": self._handle_game_over,
            "game_reset": self._handle_game_reset,
            "error": self._handle_error,
        }

        handler = handlers.get(msg_type)
        if handler:
            handler(response)
        else:
            self.logger_.warning(f"Unknown message type: {msg_type}")

    # ========== Message Handlers ==========

    def _handle_session_created(self, response: dict):
        """Handle session creation"""
        session_id = response.get("session_id")
        self.context.on_connected(session_id)
        self.console_view.display_connected(session_id)

    def _handle_join_success(self, response: dict):
        """Handle successful join"""
        color = response.get("color")
        single_player = response.get("single_player")
        status = response.get("status", "")
        session_id = response.get("session_id")

        self.context.on_joined(session_id, single_player, color)

        if not single_player:
            self.model.set_player_joined(color)
            self.console_view.display_success(f"Joined as {color}")
        else:
            # Player 1 play both colors
            self.model.set_player_joined("white")
            self.model.set_player_joined("black")
            self.console_view.display_success(f"Joined as white and black")

        if status:
            self.console_view.display_info(status)

    def _handle_player_joined(self, response: dict):
        """Handle notification that another player joined"""
        color = response.get("color")
        status = response.get("status", "")

        self.model.set_player_joined(color)

        self.console_view.display_info(f"\n>>> Another player joined as {color} <<<")
        if status:
            self.console_view.display_info(f">>> {status} <<<")

        if self.model.both_players_joined:
            self.console_view.display_info(
                ">>> Both players ready! Press Enter to refresh <<<"
            )

        self._refresh_menu()

    def _handle_game_ready(self, response: dict):
        """Handle game ready notification"""
        status = response.get("status", "Both players joined!")
        white_player = response.get("white_player")
        black_player = response.get("black_player")

        if white_player:
            self.model.set_player_joined("white")
        if black_player:
            self.model.set_player_joined("black")

        self.console_view.display_info(f"\n>>> {status} <<<")
        self.console_view.display_info(">>> Press Enter to refresh menu <<<")

        self._refresh_menu()

    def _handle_game_started(self, response: dict):
        """Handle game started"""
        self.context.on_game_started()
        self.model.start_game()

        board = response.get("board", {})

        message = (
            "1-player game started!"
            if self.context.player_number == 1
            else "2-player game started!"
        )
        self.console_view.display_success(message)

        # Display initial board
        if board:
            fen = board.get("fen", "")

            # Display board based on view mode
            if self.view_mode == ViewMode.GUI:
                # GUI displays FEN
                self.game_view.display_board(fen)
            # Console mode doesn't auto-display (only on 'd' command)

        self._refresh_menu()

    def _handle_move_result(self, response: dict):
        """Handle move result - display board and move info"""
        strike = response.get("strike", {})
        board = response.get("board", "")
        timestamp = response.get("timestamp", None)

        # Build move description
        description = self.model.build_move_description(strike)
        suffix = self.model.build_strike_suffix(strike)

        if timestamp:
            self.console_view.display_info_with_timestamp(
                description + suffix, int(timestamp)
            )
        else:
            self.console_view.display_info(description + suffix)

        # Update turn
        strike_number = strike.get("strike_number")
        if strike_number is not None:
            self.model.update_turn(strike_number + 1)

        # Update model with board data
        if board:
            fen = board.get("fen", "")

            # Display based on view mode
            if self.view_mode == ViewMode.GUI:
                # GUI auto-displays board after each move with animation
                self.game_view.display_board(fen)
            # Console mode doesn't auto-display (only on ':d' command)

        # Check for game over conditions
        if strike.get("is_checkmate"):
            self.context.on_game_over()
            self.game_view.display_game_over("Checkmate")
        elif strike.get("is_stalemate"):
            self.context.on_game_over()
            self.game_view.display_game_over("Stalemate")

    def _handle_board_display(self, response: dict):
        """Handle board display (from 'd' command)"""
        board = response.get("data", {}).get("board", {})

        if board:
            if self.view_mode == ViewMode.GUI:
                # In GUI mode, board is already displayed
                self.console_view.display_info("Board is displayed in GUI window")
            else:
                # In console mode, display ASCII board
                self.game_view.display_board(board)
        else:
            self.console_view.display_error("No board data received")

    def _handle_game_over(self, response: dict):
        """Handle game over"""
        result = response.get("result", "Unknown")

        self.context.on_game_over()

        # Display using both views
        self.game_view.display_game_over(result)

        self._refresh_menu()

    def _handle_game_reset(self, response: dict):
        """Handle game reset"""
        self.context.on_reset()
        self.model.reset()
        self.console_view.display_info("Game has been reset")

        # Close the GUI window
        if self.view_mode == ViewMode.GUI:
            self.game_view.cleanup()

        self._refresh_menu()

    def _handle_error(self, response: dict):
        """Handle error"""
        error = response.get("error", "Unknown error")
        self.console_view.display_error(error)

    def _refresh_menu(self):
        print()
        self.game_controller.show_menu()
