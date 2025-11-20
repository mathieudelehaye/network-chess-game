"""Game controller - handles user interaction and commands"""

import json
from pathlib import Path
from utils.logger import Logger
from models.client_context import ClientContext, ClientState
from models.game_model import GameModel
from views.console_view import ConsoleView


class GameController:
    """
    Game controller - handles user interaction.
    """
    
    def __init__(self, session=None):
        """
        Initialize game controller.
        
        Args:
            session: ClientSession instance for sending messages
        """

        self._logger = Logger()
        self.session = session
        self.context = ClientContext()  # gets the singleton instance
        self.model = GameModel()        # gets the singleton instance
        self.view = ConsoleView()
        
    def set_session(self, session):
        """Set session reference for sending messages"""
        self.session = session
    
    def route_message(self, message: dict):
        """
        Route incoming server response to appropriate handler.
        
        Args:
            response: JSON object from server
        """

        try:            
            response = json.loads(message)

        except json.JSONDecodeError as e:
            self._logger.error(f"Invalid JSON from server: {e}")
            self._logger.debug(f"Raw data: {message}")

        msg_type = response.get("type", "unknown")
        
        self._logger.debug(f"Routing message type: {msg_type}")
        
        # Route to handler based on type
        handlers = {
            "session_created": self._handle_session_created,
            "join_success": self._handle_join_success,
            "player_joined": self._handle_player_joined, 
            "game_ready": self._handle_game_ready,
            "game_started": self._handle_game_started,
            "move_result": self._handle_move_result,
            "board_display": self._handle_board_display,
            "game_over": self._handle_game_over,
            "error": self._handle_error,
        }
        
        handler = handlers.get(msg_type)
        if handler:
            handler(response)
        else:
            self._logger.warning(f"Unknown message type: {msg_type}")
    
    def show_menu(self):
        """Display state-aware menu"""
        state = self.context.state
        
        # Build menu info from model/context
        menu_info = {
            "state_name": self.context.get_state_name(),
            "player_color": self.context.player_color,
            "session_id": self.context.session_id,
            "current_turn": self.model.current_turn,
            "white_joined": self.model.white_joined,
            "black_joined": self.model.black_joined,
            "move_count": self.model.move_count,
        }
        
        # Display menu using view
        self.view.display_menu(menu_info)
    
    def _handle_session_created(self, response: dict):
        """Handle session creation response"""
        session_id = response.get("session_id")
        
        # Update state
        self.context.on_connected(session_id)
        
        # Update view
        self.view.display_connected(session_id)
        
    def _handle_join_success(self, response: dict):
        """Handle successful join response"""
        color = response.get("color")
        status = response.get("status", "")
        
        # Update state
        self.context.on_joined(color)
        
        # Update game data
        self.model.set_player_joined(color)
        
        # Update view
        self.view.display_success(f"Joined as {color}")
        if status:
            self.view.display_info(status)
        
    def _handle_player_joined(self, response: dict):
        """Handle notification that another player joined"""
        color = response.get("color")
        status = response.get("status", "")
        
        # Update game data
        self.model.set_player_joined(color)
        
        # Display notification
        self.view.display_info(f"\n>>> Player joined as {color} <<<")
        if status:
            self.view.display_info(f">>> {status} <<<")
        
        if self.model.both_players_joined:
            # Transition to JOINED state
            self.context.on_joined()
            self.view.display_info(">>> Both players ready! You can now start the game. <<<")
        
        # Refresh menu to show updated state
        print()  # Add newline
        self.show_menu()

    def _handle_game_ready(self, response: dict):
        """Handle notification that both players are ready"""
        status = response.get("status", "Both players joined!")
        white_player = response.get("white_player")
        black_player = response.get("black_player")
        
        # Update model with both players
        if white_player:
            self.model.set_player_joined("white")
        if black_player:
            self.model.set_player_joined("black")
        
        # Display notification
        self.view.display_info(f"\n>>> {status} <<<")
        self.view.display_info(">>> Press Enter to refresh menu <<<")
        
        # Refresh menu 
        print() 
        self.show_menu()
        print("Enter choice: ", end='', flush=True) 
        
    def _handle_game_started(self, response: dict):
        """Handle game started response"""
        # Update state
        self.context.on_game_started()
        
        # Initialize game data
        self.model.start_game()
        
        # Update view
        self.view.display_success("Game started!")
        
        # Refresh menu 
        print() 
        self.show_menu()
        print("Enter choice: ", end='', flush=True) 
        
    def _handle_move_result(self, response: dict):
        """Handle move result response"""
        strike = response.get("strike", {})
        
        # Transform data using model
        description = self.model.build_move_description(strike)
        suffix = self.model.build_strike_suffix(strike)
        full_description = description + suffix
        
        # Display using view
        self.view.display_info(full_description)
        
        # Update game data
        self.model.update_turn()
        
        # Check for game end
        if strike.get('is_checkmate'):
            self.context.on_game_over()
            self.view.display_game_over("Checkmate")
        elif strike.get('is_stalemate'):
            self.context.on_game_over()
            self.view.display_game_over("Stalemate")
            
    def _handle_board_display(self, response: dict):
        """Handle board display response"""
        board = response.get('data', {}).get('board', '')
        self.view.display_board(board)
        
    def _handle_game_over(self, response: dict):
        """Handle game over response"""
        result = response.get('result', 'Unknown')
        
        # Update state
        self.context.on_game_over()
        
        # Display result
        self.view.display_game_over(result)
        
    def _handle_error(self, response: dict):
        """Handle error response"""
        error = response.get('error', 'Unknown error')
        self.view.display_error(error)
    
    def send_join(self, color: str):
        """Send join game command"""
        if not self.context.can_join():
            self.view.display_error("Cannot join in current state")
            return
            
        self.session.send({
            "command": "join_game",
            "color": color
        })
        self._logger.info(f"Sent join command: {color}")
        
    def send_start(self):
        """Send start game command"""
        if not self.context.can_start():
            self.view.display_error("Cannot start - not in JOINED state")
            return
        
        if not self.model.both_players_joined:
            self.view.display_warning("Cannot start - waiting for both players")
            return
            
        self.session.send({
            "command": "start_game"
        })
        self._logger.info("Sent start game command")
        
    def send_move(self, from_sq: str, to_sq: str):
        """Send move command"""
        if not self.context.can_move():
            self.view.display_error("Cannot move in current state")
            return
            
        self.session.send({
            "command": "make_move",
            "from": from_sq,
            "to": to_sq
        })
        self._logger.info(f"Sent move: {from_sq}-{to_sq}")
        
    def send_display_board(self):
        """Send display board command"""
        if not self.context.can_display_board():
            self.view.display_error("Cannot display board in current state")
            return
            
        self.session.send({
            "command": "display_board"
        })
        self._logger.info("Sent display board command")
        
    def send_end_game(self):
        """Send end game command"""
        self.session.send({
            "command": "end_game"
        })
        self._logger.info("Sent end game command")
    
    def run_interactive_mode(self):
        """
        Interactive mode with state-aware menu.
        Runs main game loop.
        """
        import time

        # Wait for connection (session_created message)
        while self.context.state == ClientState.DISCONNECTED:
            time.sleep(0.1)
        
        self._logger.info("Starting interactive mode")
        
        # Main game loop
        while True:
            # Show menu (view queries model/context)
            self.show_menu()
            
            # Get user input
            choice = self.view.get_user_choice()
            
            # Handle quit
            if choice.lower() in ["quit", "q", "exit"]:
                if self.view.confirm_action("Are you sure you want to quit?"):
                    break
                continue
                
            # Handle choice
            self._handle_menu_choice(choice)
            
            # Small delay for response processing
            time.sleep(0.1)
    
    def _handle_menu_choice(self, choice: str):
        """Handle user menu choice based on current state"""
        state = self.context.state
        
        try:
            # STATE: CONNECTED (can join)
            if state == ClientState.CONNECTED:
                # Build available options dynamically
                white_available = not self.model.white_joined
                black_available = not self.model.black_joined
                both_available = white_available and black_available
                
                # Map choices to actions based on what's available
                option_num = 1
                actions = {}
                
                if both_available:
                    actions[str(option_num)] = ("single", None)
                    option_num += 1
                
                if white_available:
                    actions[str(option_num)] = ("join", "white")
                    option_num += 1
                
                if black_available:
                    actions[str(option_num)] = ("join", "black")
                    option_num += 1
                
                actions[str(option_num)] = ("upload", None)
                
                # Execute chosen action
                if choice in actions:
                    action_type, color = actions[choice]
                    
                    if action_type == "single":
                        self.send_join("white")
                        self.send_join("black")
                        self.view.display_info("Single-player mode: You control both sides")
                        
                    elif action_type == "join":
                        self.send_join(color)
                        
                    elif action_type == "upload":
                        self.view.display_warning("File upload not yet implemented")
                else:
                    self.view.display_error("Invalid choice")
                        
            # STATE: JOINED (can start)
            elif state == ClientState.JOINED:
                if choice == "1":
                    self.send_start()
                else:
                    self.view.display_error("Invalid choice")
                        
            # STATE: PLAYING (can move/display)
            elif state == ClientState.PLAYING:
                if choice == "1":
                    from_sq, to_sq = self.view.get_move_input()
                    if from_sq and to_sq:
                        self.send_move(from_sq, to_sq)
                    else:
                        self.view.display_error("Invalid move format")
                elif choice == "2":
                    self.send_display_board()
                elif choice == "3":
                    if self.view.confirm_action("End the game?"):
                        self.send_end_game()
                else:
                    self.view.display_error("Invalid choice")
                        
            # STATE: GAME_OVER
            elif state == ClientState.GAME_OVER:
                self.view.display_info("Game over. Type 'quit' to exit.")
                    
        except Exception as e:
            self._logger.error(f"Error handling choice: {e}")
            self.view.display_error(f"Error: {e}")
    
    def run_file_mode(self, 
        file_path: Path, 
        chunk_size: int = 4096):
        """
        File upload mode - upload and play game from file.
        
        Args:
            file_path: Path to game file
        """
        import time
        
        try:
            file_size = file_path.stat().st_size
            filename = file_path.name
            chunks_total = (file_size + chunk_size - 1) // chunk_size
            
            self._logger.info(f"Uploading {filename} ({file_size} bytes)")
            
            with open(file_path, 'r', encoding='utf-8') as f:
                chunk_current = 0
                
                while True:
                    chunk_data = f.read(chunk_size)
                    if not chunk_data:
                        break
                    
                    chunk_current += 1
                    
                    message = {
                        "command": "upload_game",
                        "metadata": {
                            "filename": filename,
                            "total_size": file_size,
                            "chunks_total": chunks_total,
                            "chunk_current": chunk_current
                        },
                        "data": chunk_data
                    }
                    
                    if not self._client.send(message):
                        self._logger.error(f"Failed to send chunk {chunk_current}")
                        return 1
                    
                    percent = (chunk_current * 100) // chunks_total
                    if chunk_current % 10 == 0 or chunk_current == chunks_total:
                        self._logger.info(f"Upload: {percent}%")
                    
                    time.sleep(0.01)
            
            self._logger.info(f"Upload complete: {filename}")
            return 0
            
        except Exception as e:
            self._logger.error(f"Error uploading file: {e}")
            return 1