import signal
import sys
import time
from pathlib import Path
from network.client import Client
from network.network_mode import NetworkMode
from views.menu_view import MenuView
from models.game_state import ClientGameState
from utils.logger import Logger


class GameController:
    """Main controller for chess client"""
    
    def __init__(self, host: str = "127.0.0.1", port: int = 2000, logger: Logger = None):
        self._host = host
        self._port = port
        self._logger = logger or Logger()
        self._client = Client(mode=NetworkMode.TCP, host=host, port=port)
        self._menu_view = MenuView()
        self._game_state = ClientGameState()
        self._running = False
        self._my_color = None
        self._single_player = False  # Track if in single-player mode
        
        signal.signal(signal.SIGINT, self._signal_handler)
    
    def _signal_handler(self, sig, frame):
        """Handle Ctrl+C"""
        self._logger.info("\nDisconnecting...")
        self._client.disconnect()
        sys.exit(0)
    
    def _connect(self) -> bool:
        """Connect to server"""
        self._logger.info(f"Connecting to {self._host}:{self._port}...")
        
        if not self._client.connect():
            self._logger.error("Failed to connect to server")
            return False
        
        self._logger.info("Connected to server")
        return True
    
    def _disconnect(self):
        """Disconnect from server"""
        self._logger.info("Disconnecting...")
        self._client.disconnect()
    
    # ========================================
    # INTERACTIVE MODE (Single + Multiplayer)
    # ========================================
    
    def run_interactive_mode(self) -> int:
        """Run interactive mode with menu"""
        if not self._connect():
            return 1
        
        self._running = True
        
        try:
            while self._running:
                self._update_game_status()
                self._display_menu()
                self._handle_user_input()
        except (KeyboardInterrupt, EOFError):
            self._logger.info("\nExiting...")
        finally:
            self._disconnect()
        
        return 0
    
    def _update_game_status(self):
        """Request game status from server"""
        request = {"command": "get_status"}
        self._client.send_message(request)
        
        response = self._client.receive_message()
        if response:
            self._game_state.update(response)
    
    def _display_menu(self):
        """Display menu"""
        self._menu_view.display_menu(self._game_state, self._my_color)
    
    def _handle_user_input(self):
        """Process user menu choice"""
        choice = self._menu_view.get_user_choice()
        
        # Single-player option
        if choice == "1" and not self._game_state.white_joined and not self._game_state.black_joined:
            self._handle_single_player()
        
        # Multiplayer join
        elif choice == "2":
            if not self._game_state.white_joined:
                self._handle_join_white()
            elif not self._game_state.black_joined:
                self._handle_join_black()
        
        elif choice == "3":
            if not self._game_state.white_joined:
                self._handle_join_black()
            elif self._game_state.can_start:
                self._handle_start_game()
        
        # In-game actions
        elif choice == "4":
            if self._game_state.in_game:
                self._handle_make_move()
            else:
                self._handle_upload_file()
        
        elif choice == "5":
            if self._game_state.in_game:
                self._handle_display_board()
            else:
                self._handle_exit()
        
        elif choice == "6" and self._game_state.in_game:
            self._handle_end_game()
        
        elif choice.lower() in ["exit", "quit", "q"]:
            self._handle_exit()
        
        else:
            self._logger.warning("Invalid choice")
    
    # ========================================
    # SINGLE-PLAYER MODE
    # ========================================
    
    def _handle_single_player(self):
        """Start single-player game (control both sides)"""
        self._logger.info("Starting single-player game...")
        self._single_player = True
        
        # Join as both White and Black
        # First join as White
        request = {"command": "join_game", "color": "white"}
        self._client.send_message(request)
        response = self._client.receive_message()
        
        if not response or response.get("type") != "join_success":
            self._logger.error("Failed to join as White")
            self._single_player = False
            return
        
        self._logger.info("✓ Joined as White")
        
        # Then join as Black (server needs to allow this for single-player)
        request = {"command": "join_game", "color": "black"}
        self._client.send_message(request)
        response = self._client.receive_message()
        
        if not response or response.get("type") != "join_success":
            self._logger.error("Failed to join as Black")
            self._single_player = False
            return
        
        self._logger.info("✓ Joined as Black")
        self._my_color = "both"  # Special marker for single-player
        
        # Auto-start the game
        request = {"command": "start_game"}
        self._client.send_message(request)
        response = self._client.receive_message()
        
        if response and response.get("type") == "game_started":
            self._logger.info("✓ Single-player game started!")
        else:
            self._logger.error("Failed to start game")
            self._single_player = False
    
    # ========================================
    # MULTIPLAYER MODE
    # ========================================
    
    def _handle_join_white(self):
        """Join as White player"""
        if self._game_state.white_joined:
            self._logger.warning("White slot already taken")
            return
        
        request = {"command": "join_game", "color": "white"}
        self._client.send_message(request)
        
        response = self._client.receive_message()
        if response and response.get("type") == "join_success":
            self._my_color = "white"
            self._logger.info("✓ Joined as White")
        else:
            error = response.get("error", "Unknown error") if response else "No response"
            self._logger.error(f"Failed to join: {error}")
    
    def _handle_join_black(self):
        """Join as Black player"""
        if self._game_state.black_joined:
            self._logger.warning("Black slot already taken")
            return
        
        request = {"command": "join_game", "color": "black"}
        self._client.send_message(request)
        
        response = self._client.receive_message()
        if response and response.get("type") == "join_success":
            self._my_color = "black"
            self._logger.info("✓ Joined as Black")
        else:
            error = response.get("error", "Unknown error") if response else "No response"
            self._logger.error(f"Failed to join: {error}")
    
    def _handle_start_game(self):
        """Start the game"""
        if not self._game_state.can_start:
            self._logger.warning("Cannot start game yet")
            return
        
        request = {"command": "start_game"}
        self._client.send_message(request)
        
        response = self._client.receive_message()
        if response and response.get("type") == "game_started":
            self._logger.info("✓ Game started!")
        else:
            error = response.get("error", "Unknown error") if response else "No response"
            self._logger.error(f"Failed to start: {error}")
    
    def _handle_make_move(self):
        """Make a chess move"""
        if not self._game_state.in_game:
            self._logger.warning("Game not started")
            return
        
        # In single-player, allow moves anytime
        # In multiplayer, check turn
        if not self._single_player:
            if self._game_state.current_turn and self._game_state.current_turn != self._my_color:
                self._logger.warning(f"Not your turn (waiting for {self._game_state.current_turn})")
                return
        
        from_sq, to_sq = self._menu_view.get_move_input()
        if not from_sq or not to_sq:
            return
        
        request = {
            "command": "make_move",
            "from": from_sq,
            "to": to_sq
        }
        self._client.send_message(request)
        
        response = self._client.receive_message()
        if response:
            if response.get("type") == "error":
                self._logger.error(f"Move failed: {response.get('error')}")
            else:
                self._logger.info("✓ Move applied!")
                self._menu_view.display_move_result(response)
    
    def _handle_display_board(self):
        """Display current board state"""
        request = {"command": "display_board"}
        self._client.send_message(request)
        
        response = self._client.receive_message()
        if response:
            # Display board from response
            print("\n" + "="*60)
            print("CURRENT BOARD:")
            print("="*60)
            # TODO: Format and display board
            print(response)
    
    def _handle_end_game(self):
        """End current game"""
        request = {"command": "end_game"}
        self._client.send_message(request)
        
        response = self._client.receive_message()
        if response:
            self._logger.info("Game ended")
            self._my_color = None
            self._single_player = False
    
    def _handle_exit(self):
        """Exit the client"""
        self._logger.info("Exiting...")
        self._running = False
    
    # ========================================
    # FILE UPLOAD MODE
    # ========================================
    
    def _handle_upload_file(self):
        """Upload a game file"""
        filepath = input("Enter game file path: ").strip()
        if not filepath:
            return
        
        path = Path(filepath)
        if not path.exists():
            self._logger.error(f"File not found: {filepath}")
            return
        
        self.run_file_mode(path)
    
    def run_file_mode(self, filepath: Path, chunk_size: int = 4096) -> int:
        """Upload and play a game file"""
        try:
            file_size = filepath.stat().st_size
            filename = filepath.name
            chunks_total = (file_size + chunk_size - 1) // chunk_size
            
            self._logger.info(f"Uploading {filename} ({file_size} bytes)")
            
            with open(filepath, 'r', encoding='utf-8') as f:
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
                    
                    if not self._client.send_message(message):
                        self._logger.error(f"Failed to send chunk {chunk_current}")
                        return 1
                    
                    percent = (chunk_current * 100) // chunks_total
                    if chunk_current % 10 == 0 or chunk_current == chunks_total:
                        self._logger.info(f"Upload: {percent}%")
                    
                    time.sleep(0.01)
            
            self._logger.info(f"✓ Upload complete: {filename}")
            return 0
            
        except Exception as e:
            self._logger.error(f"Error uploading file: {e}")
            return 1