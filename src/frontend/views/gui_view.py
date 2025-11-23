"""
View layer for displaying game information to the user, which are only used when
the ViewMode.GUI mode is selected. Some other console functions might be shared
between the ViewMode.NOGUI and ViewMode.GUI modes, and therefore will be
implemented by SharedConsoleView class instead.
"""

import pygame
from pathlib import Path
from typing import Optional
from views.view_interface import IView
from utils.logger import Logger


class GuiView(IView):
    """
    Pygame-based graphical view implementation.
    
    Only used for PLAYING state (board display and move input).
    Other states still use console for text input.
    """

    # Board dimensions
    SQUARE_SIZE = 80
    BOARD_SIZE = 8 * SQUARE_SIZE
    SIDEBAR_WIDTH = 200
    WINDOW_WIDTH = BOARD_SIZE + SIDEBAR_WIDTH
    WINDOW_HEIGHT = BOARD_SIZE

    # Colours
    LIGHT_SQUARE = (240, 217, 181)
    DARK_SQUARE = (181, 136, 99)
    SELECTED_COLOUR = (246, 246, 130)
    BG_COLOUR = (50, 50, 50)
    BUTTON_COLOUR = (70, 70, 70)
    BUTTON_HOVER_COLOUR = (90, 90, 90)
    BUTTON_TEXT_COLOUR = (255, 255, 255)

    def __init__(self):
        """Initialise pygame GUI - window created lazily on first display_board"""
        self.logger_ = Logger()
        
        # Pygame components (initialised lazily)
        self.screen = None
        self.font_large = None
        self.font_medium = None
        self.font_small = None
        self._initialised = False

        # Piece images
        self.piece_images = {}

        # Game state
        self.board_state = None
        self.selected_square = None

        # Button rectangles
        self.restart_button_rect = None                   
        self.quit_button_rect = None                    
        
        self.logger_.info("GUI view created (not initialised yet)")
    
    def _initialise_pygame(self):
        """Initialise pygame window and resources (called on first display_board)"""
        if self._initialised:
            return
        
        try:
            pygame.init()
            
            # Create window
            self.screen = pygame.display.set_mode((self.WINDOW_WIDTH, self.WINDOW_HEIGHT))
            pygame.display.set_caption("Chess Game")
            
            # Load fonts
            self.font_large = pygame.font.Font(None, 72)
            self.font_medium = pygame.font.Font(None, 36)
            self.font_small = pygame.font.Font(None, 24)
            
            # Load sprite sheet
            self._load_sprite_sheet()
            
            self._initialised = True
            self.logger_.info("Pygame GUI initialised successfully")
            
        except Exception as e:
            self.logger_.error(f"Failed to initialise pygame: {e}")
            raise
    
    # TODO: sprite display should be improved, since their transparency is not
    # optimal 
    def _load_sprite_sheet(self):
        """Load chess pieces from sprite sheet with transparency support"""
        # Path to sprite sheet
        sprite_path = Path(__file__).parent / "chess_sprite_sheet.png"
        
        if not sprite_path.exists():
            error_msg = f"Sprite sheet not found at {sprite_path}"
            self.logger_.error(error_msg)
            raise FileNotFoundError(error_msg)
        
        try:
            # Load the sprite sheet with alpha channel (transparency)
            sprite_sheet = pygame.image.load(str(sprite_path)).convert_alpha()
            
            # Sprite sheet layout: 6 columns x 2 rows
            # Top row (white): King, Queen, Bishop, Knight, Rook, Pawn
            # Bottom row (black): King, Queen, Bishop, Knight, Rook, Pawn
            
            # Get dimensions of each piece (assuming equal spacing)
            sheet_width = sprite_sheet.get_width()
            sheet_height = sprite_sheet.get_height()
            piece_width = sheet_width // 6
            piece_height = sheet_height // 2
            
            # Mapping of FEN symbols to sprite sheet positions (col, row)
            piece_positions = {
                'K': (0, 0),  # White King
                'Q': (1, 0),  # White Queen
                'B': (2, 0),  # White Bishop
                'N': (3, 0),  # White Knight
                'R': (4, 0),  # White Rook
                'P': (5, 0),  # White Pawn
                'k': (0, 1),  # Black King
                'q': (1, 1),  # Black Queen
                'b': (2, 1),  # Black Bishop
                'n': (3, 1),  # Black Knight
                'r': (4, 1),  # Black Rook
                'p': (5, 1),  # Black Pawn
            }
            
            # Extract each piece
            for symbol, (col, row) in piece_positions.items():
                # Calculate position in sprite sheet
                x = col * piece_width
                y = row * piece_height
                
                # Create a subsurface for this piece
                piece_rect = pygame.Rect(x, y, piece_width, piece_height)
                piece_surface = sprite_sheet.subsurface(piece_rect).copy()
                
                # Convert to per-pixel alpha for proper transparency
                piece_surface = piece_surface.convert_alpha()
                
                # Scale to fit square (with some padding) - smoothscale preserves transparency better
                scaled_piece = pygame.transform.smoothscale(
                    piece_surface, 
                    (self.SQUARE_SIZE - 10, self.SQUARE_SIZE - 10)
                )
                
                self.piece_images[symbol] = scaled_piece
            
            self.logger_.info("Loaded pieces from sprite sheet successfully")
            
        except pygame.error as e:
            error_msg = f"Failed to load sprite sheet: {e}"
            self.logger_.error(error_msg)
            raise RuntimeError(error_msg) from e

    def display_welcome(self) -> None:
        """Display welcome in console (GUI not needed here)"""
        print("\n" + "=" * 60)
        print(" " * 15 + "♔ CHESS GAME CLIENT ♔")
        print(" " * 10 + "(GUI mode will activate during play)")
        print("=" * 60 + "\n")

    def display_board(self, fen: str = None, last_move: Optional[str] = None) -> None:
        """
        Display the chess board from FEN notation
        
        Args:
            fen: FEN string representing board state
            last_move: Ignored (kept for compatibility)
        """
        # Initialise pygame on first call
        if not self._initialised:
            try:
                self._initialise_pygame()
            except Exception as e:
                self.logger_.error(f"Cannot display board - pygame initialisation failed: {e}")
                print(f"\n!!! ERROR: Cannot display GUI board - {e} !!!\n")
                return
        
        if fen:
            self.board_state = self._parse_fen(fen)
        
        self._render()

    def _parse_fen(self, fen: str):
        """Parse FEN string into 8x8 board array"""
        board = [[None for _ in range(8)] for _ in range(8)]
        
        # Extract just the piece placement part (before first space)
        piece_placement = fen.split()[0]
        
        rank = 0
        file = 0
        
        for char in piece_placement:
            if char == '/':
                rank += 1
                file = 0
            elif char.isdigit():
                file += int(char)
            else:
                board[rank][file] = char
                file += 1
        
        return board
    
    def _render(self, show_buttons: bool = False, is_single_player: bool = False) -> None:
        """Render the entire board and UI"""
        if not self._initialised or not self.screen:
            return
        
        # Handle pygame events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self.cleanup()
                return

        # Clear screen
        self.screen.fill(self.BG_COLOUR)
        
        # Draw board
        self._draw_board()
        
        # Draw pieces
        self._draw_pieces()
        
        # Draw info panel
        self._draw_info_panel(show_buttons, is_single_player)
        
        # Update display
        pygame.display.flip()

    def _draw_board(self) -> None:
        """Draw the chess board squares"""
        for row in range(8):
            for col in range(8):
                x = col * self.SQUARE_SIZE
                y = row * self.SQUARE_SIZE
                
                # Determine square colour
                is_light = (row + col) % 2 == 0
                colour = self.LIGHT_SQUARE if is_light else self.DARK_SQUARE
                
                # Highlight selected square
                if self.selected_square == (row, col):
                    colour = self.SELECTED_COLOUR
                
                pygame.draw.rect(self.screen, colour, (x, y, self.SQUARE_SIZE, self.SQUARE_SIZE))

    def _draw_pieces(self) -> None:
        """Draw chess pieces with transparency"""
        if not self.board_state:
            return
            
        for row in range(8):
            for col in range(8):
                piece = self.board_state[row][col]
                if piece and piece in self.piece_images:
                    x = col * self.SQUARE_SIZE + 5
                    y = row * self.SQUARE_SIZE + 5
                    # Blit with alpha channel preserves transparency
                    self.screen.blit(self.piece_images[piece], (x, y))

    def _draw_info_panel(self, show_buttons: bool = False, is_single_player: bool = False) -> None:
        """Draw information panel on the right side"""
        panel_x = self.BOARD_SIZE + 10
        
        # Title
        title = self.font_medium.render("Chess Game", True, (255, 255, 255))
        self.screen.blit(title, (panel_x, 20))
        
        # Instructions
        y_offset = 80
        instructions = [
            "Click to select piece",
            "Click to move",
            "",
            "Console for commands"
        ]
        
        for line in instructions:
            text = self.font_small.render(line, True, (200, 200, 200))
            self.screen.blit(text, (panel_x, y_offset))
            y_offset += 30

        # Draw buttons if in wait_for_input mode 
        if show_buttons:
            y_offset += 40
            button_width = 160
            button_height = 40

            # Restart button (only for single player)
            if is_single_player:
                self.restart_button_rect = pygame.Rect(panel_x, y_offset, button_width, button_height)
                self._draw_button(self.restart_button_rect, "Restart", pygame.mouse.get_pos())
                y_offset += button_height + 20
            else:
                self.restart_button_rect = None
            
            # Quit button (always visible)
            self.quit_button_rect = pygame.Rect(panel_x, y_offset, button_width, button_height)
            self._draw_button(self.quit_button_rect, "Quit", pygame.mouse.get_pos())  

    def _draw_button(self, rect: pygame.Rect, text: str, mouse_pos: tuple) -> None:  
        """Draw a button with hover effect"""
        # Check if mouse is hovering over button
        is_hover = rect.collidepoint(mouse_pos)
        colour = self.BUTTON_HOVER_COLOUR if is_hover else self.BUTTON_COLOUR
        
        # Draw button background
        pygame.draw.rect(self.screen, colour, rect, border_radius=5)
        pygame.draw.rect(self.screen, (100, 100, 100), rect, width=2, border_radius=5)
        
        # Draw button text (centered)
        text_surface = self.font_small.render(text, True, self.BUTTON_TEXT_COLOUR)
        text_rect = text_surface.get_rect(center=rect.center)
        self.screen.blit(text_surface, text_rect)

    def _coords_to_algebraic(self, row: int, col: int) -> str:
        """Convert board coordinates to algebraic notation"""
        file = chr(ord('a') + col)
        rank = str(8 - row)
        return file + rank

    def _algebraic_to_coords(self, algebraic: str) -> tuple:
        """Convert algebraic notation to board coordinates"""
        if len(algebraic) < 2:
            return None
        col = ord(algebraic[0]) - ord('a')
        row = 8 - int(algebraic[1])
        return (row, col)

    def wait_for_input(self, info) -> tuple[str, ...]:
        """
        Wait for user to make a move by clicking or press buttons
        
        Args:
            info: Dictionary containing game info (player_number, etc.)
        
        Returns:
            Tuple containing command:
            - ("e2-e4",) for moves
            - ("r", None) for restart
            - ("q", None) for quit
        """
        if not self._initialised:
            self.logger_.error("Cannot wait for input - pygame not initialised")
            return (":q",)
        
        # Check if single player mode           
        is_single_player = info.get('player_number', 1) == 1      

        from_square = None
        
        while True:
            mouse_pos = pygame.mouse.get_pos()

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    return (":q",)
                
                if event.type == pygame.MOUSEBUTTONDOWN:
                    x, y = event.pos
                    
                    # Check if Restart button clicked (single player only)
                    if is_single_player and self.restart_button_rect and self.restart_button_rect.collidepoint(x, y):
                        self.logger_.info("Restart button clicked")
                        print("\n>>> Restarting game <<<\n")
                        return ('r', None)
                    
                    # Check if Quit button clicked
                    if self.quit_button_rect and self.quit_button_rect.collidepoint(x, y):
                        self.logger_.info("Quit button clicked")
                        print("\n>>> Quitting game <<<\n")
                        # Quit with no confirmation
                        return ('q', True)       

                    # Check if click is on board
                    if x < self.BOARD_SIZE and y < self.BOARD_SIZE:
                        row = y // self.SQUARE_SIZE
                        col = x // self.SQUARE_SIZE
                        clicked_square = (row, col)
                        clicked_pos = self._coords_to_algebraic(row, col)
                        
                        if from_square is None:
                            # First click - select piece
                            piece = self.board_state[row][col] if self.board_state else None
                            if piece:
                                from_square = clicked_pos
                                self.selected_square = clicked_square
                                self._render()
                                
                                # Console log
                                piece_name = self._get_piece_name(piece)
                                self.logger_.info(f"Piece {piece_name} picked from square {from_square}")
                                print(f"\n>>> Piece {piece_name} picked from square {from_square} <<<")
                        else:
                            # Second click - intent to move to square
                            to_square = clicked_pos
                            self.selected_square = None
                            self._render()
                            
                            move = from_square + "-" + to_square
                            
                            # Console log
                            self.logger_.info(f"Intent to move to square {to_square}")
                            print(f">>> Move to square {to_square} (intent) <<<")
                            print(f">>> Sending move: {move} <<<\n")
                            
                            # Return as tuple for compatibility with interface
                            return ('m', move)
            
            # Re-render to update button hover effects 
            self._render(show_buttons=True, is_single_player=is_single_player)
          
            pygame.time.wait(10)

    def _get_piece_name(self, symbol):
        """Get piece name from symbol"""
        pieces = {
            'P': 'White Pawn', 'N': 'White Knight', 'B': 'White Bishop',
            'R': 'White Rook', 'Q': 'White Queen', 'K': 'White King',
            'p': 'Black Pawn', 'n': 'Black Knight', 'b': 'Black Bishop',
            'r': 'Black Rook', 'q': 'Black Queen', 'k': 'Black King',
        }
        return pieces.get(symbol, symbol)

    def display_game_over(self, result: str) -> None:
        """Display game over in both GUI and console"""
        print("\n" + "=" * 60)
        print(f" GAME OVER - {result}")
        print("=" * 60 + "\n")
        
        if not self._initialised or not self.screen:
            return
        
        # Draw semi-transparent overlay
        overlay = pygame.Surface((self.BOARD_SIZE, self.BOARD_SIZE))
        overlay.set_alpha(200)
        overlay.fill((0, 0, 0))
        self.screen.blit(overlay, (0, 0))
        
        # Draw game over text
        text1 = self.font_large.render("GAME OVER", True, (255, 255, 255))
        text2 = self.font_medium.render(result, True, (255, 200, 200))
        
        rect1 = text1.get_rect(center=(self.BOARD_SIZE // 2, self.BOARD_SIZE // 2 - 40))
        rect2 = text2.get_rect(center=(self.BOARD_SIZE // 2, self.BOARD_SIZE // 2 + 20))
        
        self.screen.blit(text1, rect1)
        self.screen.blit(text2, rect2)
        
        pygame.display.flip()

    def cleanup(self) -> None:
        """Clean up view resources"""
        if self._initialised:
            pygame.quit()
            self._initialised = False
            self.screen = None
            self.logger_.info("Pygame GUI cleaned up")