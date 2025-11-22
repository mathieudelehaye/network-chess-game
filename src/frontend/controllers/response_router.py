import json
from utils.logger import Logger
from models.client_context import ClientContext
from models.game_model import GameModel
from views.console_view import ConsoleView


class ResponseRouter:
    """
    Routes incoming server messages to appropriate handlers.
    """
    
    def __init__(
        self, 
        game_controller
    ):
        """
        Initialise message router.
        
        Args:
            game_controller: Optional reference to GameController for menu refresh
        """

        self.logger_ = Logger()
        self.context = ClientContext()  # gets the singleton instance
        self.model = GameModel()        # gets the singleton instance
        self.view = ConsoleView()
        self.game_controller = game_controller
    
    def route(self, message: str):
        """Route incoming server message to appropriate handler"""
        
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
    
    # Message handlers
    def _handle_session_created(self, response: dict):
        """Handle session creation"""
        session_id = response.get("session_id")
        self.context.on_connected(session_id)
        self.view.display_connected(session_id)
        
    def _handle_join_success(self, response: dict):
        """Handle successful join"""
        color = response.get("color")
        single_player = response.get("single_player")
        status = response.get("status", "")
        session_id = response.get("session_id")
        
        self.context.on_joined(session_id, single_player, color)

        if not single_player:
            self.model.set_player_joined(color)
            self.view.display_success(f"Joined as {color}")
        else:
            # Player 1 play both colors  
            self.model.set_player_joined("white")
            self.model.set_player_joined("black")
            self.model.start_game()
            self.view.display_success(f"Joined as white and black")
        
        if status:
            self.view.display_info(status)
        
    def _handle_player_joined(self, response: dict):
        """Handle notification that another player joined"""
        color = response.get("color")
        status = response.get("status", "")
        
        self.model.set_player_joined(color)
        
        self.view.display_info(f"\n>>> Another player joined as {color} <<<")
        if status:
            self.view.display_info(f">>> {status} <<<")
        
        if self.model.both_players_joined:
            self.view.display_info(">>> Both players ready! Press Enter to refresh <<<")
        
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
        
        self.view.display_info(f"\n>>> {status} <<<")
        self.view.display_info(">>> Press Enter to refresh menu <<<")
        
        self._refresh_menu()
        
    def _handle_game_started(self, _):
        """Handle game started"""
        self.context.on_game_started()
        self.model.start_game()
        
        message = "1-player game started!" if self.context.player_number == 1 else "2-player game started!"
        self.view.display_success(message)
        
        self._refresh_menu()
        
    def _handle_move_result(self, response: dict):
        """Handle move result"""
        strike = response.get("strike", {})
        
        description = self.model.build_move_description(strike)
        suffix = self.model.build_strike_suffix(strike)
        self.view.display_info(description + suffix)
        
        # Update turn to (1 + last move received from the server)
        self.model.update_turn(strike.get("strike_number", None)+1)
        
        if strike.get('is_checkmate'):
            self.context.on_game_over()
            self.view.display_game_over("Checkmate")
        elif strike.get('is_stalemate'):
            self.context.on_game_over()
            self.view.display_game_over("Stalemate")
            
    def _handle_board_display(self, response: dict):
        """Handle board display"""
        board = response.get('data', {}).get('board', '')
        self.view.display_board(board)
        
    def _handle_game_over(self, response: dict):
        """Handle game over"""
        result = response.get('result', 'Unknown')
        self.context.on_game_over()
        self.view.display_game_over(result)
    
    def _handle_game_reset(self, response: dict):
        """Handle game reset"""
        self.context.on_reset()
        self.model.reset()
        self.view.display_info("Game has been reset")
        
        self._refresh_menu()
        
    def _handle_error(self, response: dict):
        """Handle error"""
        error = response.get('error', 'Unknown error')
        self.view.display_error(error)

    def _refresh_menu(self):
        print()
        self.game_controller.show_menu()