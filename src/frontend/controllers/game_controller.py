"""Game controller - handles user interaction and commands.

Manages game flow, user input, and coordination between model,
context, and views. Supports both interactive and file-based modes.
"""

import os
import time
from pathlib import Path
from models.client_context import ClientContext, ClientState
from models.game_model import GameModel
from utils.logger import Logger
from views.view_interface import IView
from views.view_factory import ViewMode
from views.shared_console_view import SharedConsoleView

class GameController:
    """Game controller with dual-view support.
    
    Coordinates game logic, user interaction, and display using
    SharedConsoleView for common I/O and IView for game-specific display.
    Manages both interactive and file-based game modes.
    """
    
    def __init__(self, view_mode: ViewMode, game_view: IView, console_view: SharedConsoleView):
        """Initialize controller with view objects.
        
        Args:
            view_mode: Display mode (GUI or console)
            game_view: View for game display (NoGUIConsoleView or GuiView)
            console_view: Shared console view for common I/O operations
        """

        self._context = ClientContext()
        self._model = GameModel()

        self.view_mode = view_mode
        self._game_view = game_view          # IView implementation (GUI or Console)
        self._console_view = console_view    # SharedConsoleView for menus/messages
        
        self._logger_ = Logger()
        self._session = None

    @property
    def context(self) -> ClientContext:
        return self._context

    @property
    def model(self) -> GameModel:
        return self._model
    
    @property
    def game_view(self) -> IView:
        return self._game_view
    
    @property
    def console_view(self) -> IView:
        return self._console_view
        
    def set_session(self, session):
        """Set session reference for sending messages.
        
        Args:
            session: ClientSession instance for server communication
        """
        self._session = session
    
    def run_interactive_mode(self):
        """Run main interactive game loop.
        
        Displays welcome, waits for connection, shows menu,
        handles user input, and processes commands until quit.
        """
        self._logger_.info("Starting interactive mode")

        # Show welcome using game view
        self._game_view.display_welcome()

        # Wait for connection (session_created message)
        while self._context.state == ClientState.DISCONNECTED:
            time.sleep(0.1)
        
        self._logger_.info("Connected - starting game loop")
        
        # Main game loop
        while True:
            # Show menu
            self.show_menu()
            
            # Get user input
            while True:
                choice = self.wait_for_input()
                if choice[0]:
                    break

            # Handle quit
            if choice[0].lower() == 'q':
                if choice[0]:
                    # Quit with no confirmation
                    break
                if self._console_view.confirm_action("Are you sure you want to quit?"):
                    break
                continue
                
            # Handle choice
            self._handle_menu_choice(choice)
            
            # Small delay for response processing
            time.sleep(0.1)
    
    # TODO: implement a waiting mechanism, because for now the app will upload
    # the game file, then return to the main game loop (if interactive mode is
    # used), and the main menu will be displayed then polluted by the
    # move_result responses from the server. So, client should wait until all
    # move_result responses are received (or a timeout has ellapsed) before
    # re-displaying the menu.
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
        self._logger_.debug(f"File path: {file_path}")

        if not os.path.isfile(file_path):
            self._logger_.warning(f"File doesn't exist: {file_path}")
            return False
        
        try:
            file_size = file_path.stat().st_size
            filename = file_path.name
            chunks_total = (file_size + chunk_size - 1) // chunk_size
            
            self._logger_.info(f"Uploading {filename} ({file_size} bytes)")
            
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
                    
                    if not self._session.send(message):
                        self._logger_.error(f"Failed to send chunk {chunk_current}")
                        return 1
                    
                    percent = (chunk_current * 100) // chunks_total
                    if chunk_current % 10 == 0 or chunk_current == chunks_total:
                        self._logger_.info(f"Upload: {percent}%")
                    
                    time.sleep(0.01)
            
            self._logger_.info(f"Upload complete: {filename}")
            return True
            
        except Exception as e:
            self._logger_.error(f"Error uploading file: {e}")
            self._console_view.display_error(f"Upload error: {e}")
            return False
    
    def show_menu(self):
        """Display adaptable menu"""

        info = {
            "state_name": self._context.get_state_name(),
            "white_joined": self._model.white_joined,
            "black_joined": self._model.black_joined,
            "current_turn": self._model.current_turn,
            "move_count": self._model.move_count,
            "player_color": self._context.player_color,
            "session_id": self._context.session_id,
            "player_number": self._context.player_number,
            "gui_mode": self.view_mode == ViewMode.GUI,
        }

        return self._console_view.display_menu(info)

    def wait_for_input(self) -> tuple[str, ...]:
        """Display adaptable input prompt"""

        state_name = self._context.get_state_name()

        menu_info = {
            "state_name": state_name,
            "player_number": self._context.player_number,
            "white_joined": self._model.white_joined,
            "black_joined": self._model.black_joined,
            "current_turn": self._model.current_turn,
            "move_count": self._model.move_count,
            "gui_mode": self.view_mode == ViewMode.GUI,
        }

        if state_name == "PLAYING":
            return self._game_view.wait_for_input(menu_info)
        else:
            return self._console_view.wait_for_user_choice()
        
    def send_join(self, color: str):
        """Send join game command"""
        if not self._context.can_join():
            self._console_view.display_error("Cannot join in current state")
            return
            
        self._session.send({
            "command": "join_game",
            "single_player": False,
            "color": color
        })
        self._logger_.info(f"Sent join command: {color}")
        
    def send_join_single_player(self):
        """Send join game command for single player mode"""
        if not self._context.can_join():
            self._console_view.display_error("Cannot join in current state")
            return
            
        self._session.send({
            "command": "join_game",
            "single_player": True,
            "color": ""
        })
        self._logger_.info(f"Sent join command for single player")
        
    def send_start(self):
        """Send start game command"""
        if not self._context.can_start():
            self._console_view.display_error("Cannot start - not in JOINED state")
            return
        
        if not self._model.both_players_joined:
            self._console_view.display_warning("Cannot start - waiting for both players")
            return
            
        self._session.send({
            "command": "start_game"
        })
        self._logger_.info("Sent start game command")
        
    def send_move(self, move: str):
        """Send move command"""
        if not self._context.can_move():
            self._console_view.display_error("Cannot move in current state")
            return
            
        self._session.send({
            "command": "make_move",
            "move": move
        })
        self._logger_.debug(f"Sent move: {move}")
        
    def send_display_board(self):
        """Send display board command"""
        if not self._context.can_display_board():
            self._console_view.display_error("Cannot display board in current state")
            return
            
        self._session.send({
            "command": "display_board"
        })
        self._logger_.info("Sent display board command")
        
    def send_end_game(self):
        """Send end game command"""
        self._session.send({
            "command": "end_game"
        })
        self._logger_.info("Sent end game command")

        """Handle user menu choice based on current state"""
    def _handle_menu_choice(self, choice: tuple[str, ...]):
        state = self._context.state
        
        try:
            # STATE: CONNECTED (can join)
            if state == ClientState.CONNECTED:
                # Build available options dynamically
                white_available = not self._model.white_joined
                black_available = not self._model.black_joined
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
                        self._console_view.display_info("Single-player mode: You control both sides")
                        
                    elif action_type == "join":
                        self.send_join(color)

                else:
                    self._console_view.display_error("Invalid choice")
                        
            # STATE: JOINED (can start)
            elif state == ClientState.JOINED:
                if choice[0] == "1":
                    self.send_start()
                else:
                    self._console_view.display_error("Invalid choice")
                        
            # STATE: PLAYING (can move/use special command)
            elif state == ClientState.PLAYING:
                # special commands
                if choice[0] == 'd':
                    # display board
                    self.send_display_board()
                elif self._context.player_number == 1 and choice[0] == 'r':
                    # end game
                    self.send_end_game()
                elif self._context.player_number == 1 and choice[0] == 'f':
                    # upload game
                    self.run_file_mode(choice[1])
                elif choice[0] == 'm':
                    # regular move
                    self.send_move(choice[1])
                        
            # STATE: GAME_OVER
            elif state == ClientState.GAME_OVER:
                self._console_view.display_info("Game over. Type '1' to restart or 'q' to exit.")
                if choice[0] == "1":
                    self.send_end_game()
                    
        except Exception as e:
            self._logger_.error(f"Error handling choice: {e}")
            self._console_view.display_error(f"Error: {e}")
    
    