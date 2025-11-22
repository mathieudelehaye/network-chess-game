"""Game controller - handles user interaction and commands"""

import os
import time
from pathlib import Path
from utils.logger import Logger
from models.client_context import ClientContext, ClientState
from models.game_model import GameModel
from views.console_view import ConsoleView


class GameController:
    """
    Game controller - handles user interaction.
    """
    
    def __init__(self):
        """
        Initialise game controller.
        
        Args:
            session: ClientSession instance for sending messages
        """

        self.logger_ = Logger()
        self._last_displayed_info = None
        self.context = ClientContext()  # gets the singleton instance
        self.model = GameModel()        # gets the singleton instance
        self.view = ConsoleView()
        
    def set_session(self, session):
        """Set session reference for sending messages"""
        self.session = session
    
    def show_menu(self):
        """Display adaptable menu"""
        
        current_info = {
            "state_name": self.context.get_state_name(),
            "white_joined": self.model.white_joined,
            "black_joined": self.model.black_joined,
            "current_turn": self.model.current_turn,
            "move_count": self.model.move_count,
            "player_color": self.context.player_color,
            "session_id": self.context.session_id,
            "player_number": self.context.player_number,
        }
        
        self.view.display_menu(current_info)

    def wait_for_input(self) -> tuple[str, ...]:
        """Display adaptable input prompt"""

        menu_info = {
            "state_name": self.context.get_state_name(),
            "player_number": self.context.player_number,
        }

        return self.view.wait_for_input(menu_info)
        
    def send_join(self, color: str):
        """Send join game command"""
        if not self.context.can_join():
            self.view.display_error("Cannot join in current state")
            return
            
        self.session.send({
            "command": "join_game",
            "single_player": False,
            "color": color
        })
        self.logger_.info(f"Sent join command: {color}")
        
    def send_join_single_player(self):
        """Send join game command for single player mode"""
        if not self.context.can_join():
            self.view.display_error("Cannot join in current state")
            return
            
        self.session.send({
            "command": "join_game",
            "single_player": True,
            "color": ""
        })
        self.logger_.info(f"Sent join command for single player")
        
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
        self.logger_.info("Sent start game command")
        
    def send_move(self, move: str):
        """Send move command"""
        if not self.context.can_move():
            self.view.display_error("Cannot move in current state")
            return
            
        self.session.send({
            "command": "make_move",
            "move": move
        })
        self.logger_.debug(f"Sent move: {move}")
        
    def send_display_board(self):
        """Send display board command"""
        if not self.context.can_display_board():
            self.view.display_error("Cannot display board in current state")
            return
            
        self.session.send({
            "command": "display_board"
        })
        self.logger_.info("Sent display board command")
        
    def send_end_game(self):
        """Send end game command"""
        self.session.send({
            "command": "end_game"
        })
        self.logger_.info("Sent end game command")
    
    def run_interactive_mode(self):
        """
        Interactive mode with state-aware menu.
        Runs main game loop.
        """
        import time

        # Wait for connection (session_created message)
        while self.context.state == ClientState.DISCONNECTED:
            time.sleep(0.1)
        
        self.logger_.info("Starting interactive mode")
        
        # Main game loop
        while True:
            # Show menu
            self.show_menu()
            
            # Get user input
            choice = self.wait_for_input()
            
            # Handle quit
            if choice[0].lower() == 'q':
                if self.view.confirm_action("Are you sure you want to quit?"):
                    break
                continue
                
            # Handle choice
            self._handle_menu_choice(choice)
            
            # Small delay for response processing
            time.sleep(0.1)
    
    def _handle_menu_choice(self, choice: tuple[str, ...]):
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
                if choice[0] in actions:
                    action_type, color = actions[choice[0]]
                    
                    if action_type == "single":
                        self.send_join_single_player()
                        self.view.display_info("Single-player mode: You control both sides")
                        
                    elif action_type == "join":
                        self.send_join(color)

                else:
                    self.view.display_error("Invalid choice")
                        
            # STATE: JOINED (can start)
            elif state == ClientState.JOINED:
                if choice[0] == "1":
                    self.send_start()
                else:
                    self.view.display_error("Invalid choice")
                        
            # STATE: PLAYING (can move/use special command)
            elif state == ClientState.PLAYING:
                print(choice)

                # special commands
                if choice[0] == 'd':
                    # display board
                    pass
                    # TODO: implement displaying board
                    # self.send_display_board()
                elif choice[0] == 'f':
                    # upload game
                    self.run_file_mode(choice[1])
                elif choice[0] == 'm':
                    # regular move
                    self.send_move(choice[1])
                        
            # STATE: GAME_OVER
            elif state == ClientState.GAME_OVER:
                self.view.display_info("Game over. Type 'quit' to exit.")
                    
        except Exception as e:
            self.logger_.error(f"Error handling choice: {e}")
            self.view.display_error(f"Error: {e}")
    
    def run_file_mode(self, 
        file_path: Path, 
        chunk_size: int = 4096) -> bool:
        """
        File upload mode - upload and play game from file.
        
        Args:
            file_path: Path to game file
        """

        # Ensure `file_path` is a Path object
        file_path = Path(file_path)
        
        file_path = (Path.cwd() / file_path).resolve()
        self.logger_.debug(f"File path: {file_path}")

        if not os.path.isfile(file_path):
            self.logger_.warning(f"File doesn't exist: {file_path}")
            return False
        
        try:
            file_size = file_path.stat().st_size
            filename = file_path.name
            chunks_total = (file_size + chunk_size - 1) // chunk_size
            
            self.logger_.info(f"Uploading {filename} ({file_size} bytes)")
            
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
                    
                    if not self.session.send(message):
                        self.logger_.error(f"Failed to send chunk {chunk_current}")
                        return 1
                    
                    percent = (chunk_current * 100) // chunks_total
                    if chunk_current % 10 == 0 or chunk_current == chunks_total:
                        self.logger_.info(f"Upload: {percent}%")
                    
                    time.sleep(0.01)
            
            self.logger_.info(f"Upload complete: {filename}")
            return True
            
        except Exception as e:
            self.logger_.error(f"Error uploading file: {e}")
            return False