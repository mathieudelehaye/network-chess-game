"""
Client-side game model.
Handles game data and data transformation logic.
State management is handled by ClientContext.
"""

from typing import Optional
from utils.logger import Logger


class GameModel:
    """
    Model layer - stores game data and provides transformation logic.
    
    Responsibilities:
    1. Store game data (turn tracking, player info)
    2. Transform server data into human-readable formats
    
    State FSM is handled by ClientContext separately.
    NO parsing or validation - server does that.
    """

    _instance = None

    def __new__(cls):
        """Ensure only one instance exists (Singleton)"""
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
    
    def __init__(self):
        # Game data (not state - that's in ClientContext)
        self._move_count = 0
        self._current_turn: Optional[str] = None
        self._white_joined = False
        self._black_joined = False
    
    @property
    def move_count(self) -> int:
        return self._move_count

    @property
    def current_turn(self) -> bool:
        return self._current_turn

    @property
    def white_joined(self) -> bool:
        return self._white_joined
    
    @property
    def black_joined(self) -> bool:
        return self._black_joined
    
    @property
    def both_players_joined(self) -> bool:
        """Check if both players have joined"""
        return self._white_joined and self._black_joined
    
    def set_player_joined(self, color: str):
        """Track when a player joins"""
        if color == "white":
            self._white_joined = True
        elif color == "black":
            self._black_joined = True
    
    def start_game(self):
        """Initialise game data when game starts"""
        self._move_count = 1
        self._current_turn = "white"  # White always starts
    
    def update_turn(self, value: Optional[str]):
        """Update turn after a move"""
        self._move_count = int(value) if value else 0
        self._current_turn = "white" if self._move_count % 2 == 1 else "black"
    
    def reset(self):
        """Reset all game data"""
        self._move_count = 1
        self._current_turn = "white"
        self._white_joined = False
        self._black_joined = False
    
    @staticmethod
    def build_move_description(strike: dict) -> str:
        """
        Build human-readable move description from strike data.
        
        Args:
            strike: Strike data from server (already validated/parsed)
            
        Returns:
            Human-readable move description
        """
        logger = Logger()
        logger.debug(f"Received strike: {strike}")

        msg = f"{strike['strike_number']}. {strike['color']} {strike['piece']}"

        if strike.get("is_castling"):
            msg += f" does a {strike['castling_type']} castling"
            msg += f" from {strike['case_src']} to {strike['case_dest']}"
        elif strike.get("is_capture", False):
            msg += f" on {strike['case_src']}"
            msg += f" takes {strike['captured_color']} {strike['captured_piece']}"
            msg += f" on {strike['case_dest']}"
        else:
            msg += f" moves from {strike['case_src']} to {strike['case_dest']}"

        logger.debug(f"Generated message: {msg}")

        return msg
    
    @staticmethod
    def build_strike_suffix(strike: dict) -> str:
        """Build check/checkmate/stalemate suffix"""
        if strike.get("is_checkmate"):
            return ". Checkmate"
        elif strike.get("is_check"):
            return ". Check"
        elif strike.get("is_stalemate"):
            return ". Stalemate"
        return ""