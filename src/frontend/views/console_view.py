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
        self._logger = Logger()

    def display_menu(self, info: dict):
        """Display main menu based on game state"""
        print("\n" + "="*60)
        print("CHESS GAME CLIENT")
        print("="*60)
        
        # Extract info
        state_name = info.get('state_name', 'Unknown')
        player_color = info.get('player_color')
        current_turn = info.get('current_turn')
        white_joined = info.get('white_joined', False)
        black_joined = info.get('black_joined', False)
        move_count = info.get('move_count', 0)
        
        # Show current status
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
            print("1. Make Move")
            print("2. Display Board")
            print("3. End Game")
            print("Q. Quit")
            
        # STATE: GAME_OVER
        elif state_name == "GAME_OVER":
            print("Game Over!")
            print("Q. Quit")
        
        print("="*60)
    
    def get_user_choice(self) -> str:
        """Get user menu choice"""
        return input("\nEnter choice: ").strip()
    
    def get_move_input(self) -> tuple:
        """Get move from user (e.g., 'e2-e4')"""
        move = input("Enter move (e.g., e2-e4): ").strip()
        if '-' in move:
            parts = move.split('-')
            if len(parts) == 2:
                return parts[0].strip(), parts[1].strip()
        return None, None
    
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
        self._logger.info(f"Game Over: {result}")
    
    def display_success(self, message: str) -> None:
        """Display success message"""
        self._logger.info(message)
    
    def display_error(self, error: str) -> None:
        """Display error message"""
        self._logger.error(error)
    
    def display_info(self, message: str) -> None:
        """Display informational message"""
        self._logger.info(message)
    
    def display_warning(self, message: str) -> None:
        """Display warning message"""
        self._logger.warning(message)
    
    def display_connected(self, session_id: str) -> None:
        """Display connection success"""
        self._logger.info(f"Connected with session: {session_id}")
    
    def confirm_action(self, message: str) -> bool:
        """Ask user to confirm an action"""
        response = input(f"{message} (yes/no): ").strip().lower()
        return response in ['yes', 'y']