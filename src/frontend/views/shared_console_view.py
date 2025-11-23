"""
View layer for displaying game information to the user, which is shared between
the GUI and non-GUI (i.e. console substitute for GUI) views. Some functions
might stay in console mode anyway, although the amount of these could decrease
in the future. 
"""

from utils.logger import Logger


class SharedConsoleView:
    """
    Console text-based view.
    Also contains all shared menu/input methods.
    """

    def __init__(self):
        self.logger_ = Logger()
        
    def display_menu(self, info: dict):
        """Display main menu based on game state"""
        print("\n" + "="*60)
        print("CHESS GAME CLIENT")
        print("="*60)
        
        state_name = info.get('state_name', 'Unknown')
        player_color = info.get('player_color')
        current_turn = info.get('current_turn')
        white_joined = info.get('white_joined', False)
        black_joined = info.get('black_joined', False)
        move_count = info.get('move_count', 0)
        player_number = info.get('player_number', 1)
        gui_mode = info.get('gui_mode', False)
        
        print(f"\nStatus: {state_name}")
        
        # Show players
        if white_joined or black_joined:
            print(f"White player: {'Joined' if white_joined else 'Waiting...'}")
            print(f"Black player: {'Joined' if black_joined else 'Waiting...'}")
        
        if player_color:
            print(f"You are playing as: {player_color.upper()}")
        
        # Show turn info if playing
        if current_turn:
            turn_msg = f"Current turn: {current_turn.upper()}"
            if player_color == current_turn:
                turn_msg += " (YOUR TURN)"
            print(f"\n{turn_msg}")
            print(f"Move count: {move_count}")
        
        # Don't show the options in GUI mode
        # TODO: improve integration
        if gui_mode and state_name == "PLAYING":
            return

        print("\n" + "-"*60)
        print("MENU OPTIONS:")
        print("-"*60)
        
        # STATE: CONNECTED - Show join options
        if state_name == "CONNECTED":
            option_num = 1
            
            # Single player only if no one has joined yet
            if not white_joined and not black_joined:
                print(f"{option_num}. Single Player Game (play both sides)")
                option_num += 1
            
            # Join as white if white not taken
            if not white_joined:
                print(f"{option_num}. Join as White Player")
                option_num += 1
            
            # Join as black if black not taken
            if not black_joined:
                print(f"{option_num}. Join as Black Player")
                option_num += 1
            
            print("Q. Quit")
            
        # STATE: JOINED - Waiting for game start
        elif state_name == "JOINED":
            if white_joined and black_joined:
                print("1. Start Game")
            else:
                print("Waiting for opponent to join...")
            print("Q. Quit")
            
        # STATE: PLAYING - Game in progress
        elif state_name == "PLAYING":
            # PLAYING - state menu only displayed in non-GUI mode
            print("Waiting for the next strike.")
            print()
            print("Special commands:")
            if player_number == 1:
                print("  :r             => restart game")
                print("  :f <file name> => upload game file")
            print("  :d             => display board")
            print("  :q             => quit")
            
        # STATE: GAME_OVER
        elif state_name == "GAME_OVER":
            print("1. Restart Game")
            print("Q. Quit")
        
        print("="*60)

    def display_info(self, message: str) -> None:
        """Display info message"""
        self.logger_.info(message)

    def display_warning(self, message: str) -> None:
        """Display warning message"""
        self.logger_.warning(message)
    
    def display_error(self, error: str) -> None:
        """Display error message"""
        self.logger_.error(error)
    
    def display_success(self, message: str) -> None:
        """Display success message"""
        self.logger_.info(message)
    
    def display_connected(self, session_id: str) -> None:
        """Display connection success"""
        self.logger_.info(f"Connected with session: {session_id}")
    
    def confirm_action(self, message: str) -> bool:
        """Ask user to confirm an action"""
        response = input(f"{message} (yes/no): ").strip().lower()
        return response in ['yes', 'y']
    
    def wait_for_user_choice(self) -> tuple[str, ...]:
        """Get user menu choice"""
        return (input("\nEnter choice: ").strip(),)