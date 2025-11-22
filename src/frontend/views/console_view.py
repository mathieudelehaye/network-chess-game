"""
View layer for displaying game information to the user.
"""

from utils.logger import Logger

class ConsoleView:
    """
    Handles all user-facing output.
    Separates presentation logic from business logic.
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
            print("Game Over!")
            print("Q. Quit")
        
        print("="*60)

    def wait_for_input(self, info: dict) -> tuple[str, ...]:
        """Display prompt and wait for user input based on game state"""
        
        state_name = info.get('state_name', 'Unknown')
        player_number = info.get('player_number', 1)
        
        if state_name == "PLAYING":
            return self.get_game_input(player_number == 1)
        else:
            return (self.get_user_choice())
    
    def get_game_input(self, single_player: bool) -> tuple[str, ...]:
        """Get move from user (e.g., 'e2-e4'). Special commands
        are also allowed, including:
          - `:r                 => restart (only in single player mode)
          - `:f <file name>`    => upload game file (only in single player mode)
          - `:d`                => display board
          - `:q`                => quit the game
        """
        command = input("Enter move (or special command): ").strip()

        # Handle empty input
        if not command:
            return (None, None)
        
        # Handle special commands
        if command[0] == ':':
            if len(command) < 2:
                self.display_error("Invalid command")
                return (None, None)
        
            if single_player:
                if command[1] == 'r':
                    return ('r', None)
                if command[1] == 'f':
                    pos = command[2:].find(' ')
                    if pos == -1:
                        self.display_error("Invalid file command. Usage: :f <file name>")
                        return (None, None)
                    file_path = command[2 + pos:].strip()
                    return ('f', file_path)
            
            if command[1] == 'd':
                return ('d', None)
            
            if command[1] == 'q':
                return ('q', None)
            
        # Handle regular move
        if '-' in command:
            return ('m', command)
        
        self.display_error("Invalid input format")
        return (None, None)
    
    def get_user_choice(self) -> str:
        """Get user menu choice"""
        return input("\nEnter choice: ").strip()
    
    def display_board(self, board: str) -> None:
        """Display chess board"""
        print("\n" + "="*50)
        print("Current Board:")
        print("="*50)
        print(board)
        print("="*50 + "\n")

    def display_game_over(self, result: str) -> None:
        """Display game over message"""
        print("\n" + "="*60)
        print(f"GAME OVER: {result.upper()}")
        print("="*60 + "\n")
        self.logger_.info(f"Game Over: {result}")
    
    def display_success(self, message: str) -> None:
        """Display success message"""
        self.logger_.info(message)
    
    def display_error(self, error: str) -> None:
        """Display error message"""
        self.logger_.error(error)
    
    def display_info(self, message: str) -> None:
        """Display informational message"""
        self.logger_.info(message)
    
    def display_warning(self, message: str) -> None:
        """Display warning message"""
        self.logger_.warning(message)
    
    def display_connected(self, session_id: str) -> None:
        """Display connection success"""
        self.logger_.info(f"Connected with session: {session_id}")
    
    def confirm_action(self, message: str) -> bool:
        """Ask user to confirm an action"""
        response = input(f"{message} (yes/no): ").strip().lower()
        return response in ['yes', 'y']