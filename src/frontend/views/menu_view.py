class MenuView:
    """Display interactive menu for chess game"""
    
    def display_menu(self, game_state, my_color=None):
        """Display main menu based on game state"""
        print("\n" + "="*60)
        print("CHESS GAME CLIENT")
        print("="*60)
        
        # Show current status
        status = game_state.status_message
        print(f"\nStatus: {status}")
        
        if game_state.white_joined or game_state.black_joined:
            print(f"White player: {'✓ Joined' if game_state.white_joined else 'Waiting...'}")
            print(f"Black player: {'✓ Joined' if game_state.black_joined else 'Waiting...'}")
        
        if my_color:
            print(f"You are playing as: {my_color.upper()}")
        
        print("\n" + "-"*60)
        print("MENU OPTIONS:")
        print("-"*60)
        
        # Show appropriate menu based on state
        if not game_state.white_joined and not game_state.black_joined:
            # No one joined yet - show all options
            print("1. Single Player Game (play both sides)")
            print("2. Join as White Player (multiplayer)")
            print("3. Join as Black Player (multiplayer)")
            print("4. Upload Game File")
            print("5. Exit")
            
        elif not game_state.can_start and not game_state.in_game:
            # Waiting for second player
            if game_state.white_joined and not game_state.black_joined:
                print("2. Join as Black Player")
            elif game_state.black_joined and not game_state.white_joined:
                print("2. Join as White Player")
            print("4. Upload Game File")
            print("5. Cancel and Exit")
            
        elif game_state.can_start:
            # Both players ready
            print("3. Start Game")
            print("5. Cancel and Exit")
            
        elif game_state.in_game:
            # Game in progress
            if game_state.current_turn:
                turn_msg = f"Current turn: {game_state.current_turn.upper()}"
                if my_color == game_state.current_turn:
                    turn_msg += " (YOUR TURN)"
                print(f"\n{turn_msg}")
            
            print("4. Make Move")
            print("5. Display Board")
            print("6. End Game")
        
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
    
    def display_move_result(self, response: dict):
        """Display result of a move"""
        if response.get("type") == "move_success":
            data = response.get("data", {})
            print(f"\n✓ Move applied: {data.get('move_notation', '')}")
            
            if data.get("is_check"):
                print("⚠️  CHECK!")
            if data.get("is_checkmate"):
                print("🏆 CHECKMATE! Game Over!")
            if data.get("is_stalemate"):
                print("🤝 STALEMATE! Game Over!")