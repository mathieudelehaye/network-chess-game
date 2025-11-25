"""Client-side game model for chess application.

Handles game data and data transformation logic.
State management is handled by ClientContext.
"""

import re
from typing import Optional
from utils.logger import Logger


class GameModel:
    """Model layer - stores game data and provides transformation logic.

    Singleton class managing game data (turn tracking, player info)
    and data transformations (move descriptions). State FSM is handled
    by ClientContext separately. No parsing or validation - server does that.
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
        """Track when a player joins.

        Args:
            color: Player color ("white" or "black")
        """
        if color == "white":
            self._white_joined = True
        elif color == "black":
            self._black_joined = True

    def start_game(self):
        """Initialize game data when game starts.

        Sets move count to 1 and turn to white.
        """
        self._move_count = 1
        self._current_turn = "white"  # White always starts

    def update_turn(self, value: Optional[str]):
        """Update turn after a move.

        Args:
            value: Move number string from server
        """
        self._move_count = int(value) if value else 0
        self._current_turn = "white" if self._move_count % 2 == 1 else "black"

    def reset(self):
        """Reset all game data to initial state."""
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
        """Build check/checkmate/stalemate suffix.

        Args:
            strike: Strike data from server

        Returns:
            str: Status suffix (empty, ". Check", ". Checkmate", or ". Stalemate")
        """
        if strike.get("checkmate"):
            return ". Checkmate"
        elif strike.get("check"):
            return ". Check"
        elif strike.get("stalemate"):
            return ". Stalemate"
        return ""
