from enum import Enum, auto
from typing import Optional


class ClientState(Enum):
    """Client connection and game states"""
    DISCONNECTED = auto()
    CONNECTED = auto()
    JOINED = auto()      # Joined as a player, waiting for game start
    PLAYING = auto()     # Game in progress
    GAME_OVER = auto()


class ClientContext:
    """
    Lightweight FSM for client state
    Simpler than server - just tracks what UI should show
    """
    
    def __init__(self):
        self._state = ClientState.DISCONNECTED
        self._player_color: Optional[str] = None
        self._session_id: Optional[str] = None
        
    @property
    def state(self) -> ClientState:
        return self._state
        
    @property
    def player_color(self) -> Optional[str]:
        return self._player_color
        
    @property
    def session_id(self) -> Optional[str]:
        return self._session_id
    
    # State transitions
    def on_connected(self, session_id: str):
        """Transition: DISCONNECTED → CONNECTED"""
        if self._state == ClientState.DISCONNECTED:
            self._state = ClientState.CONNECTED
            self._session_id = session_id
            
    def on_joined(self, color: str):
        """Transition: CONNECTED → JOINED"""
        if self._state == ClientState.CONNECTED:
            self._state = ClientState.JOINED
            self._player_color = color
            
    def on_game_started(self):
        """Transition: JOINED → PLAYING"""
        if self._state == ClientState.JOINED:
            self._state = ClientState.PLAYING
            
    def on_game_over(self):
        """Transition: PLAYING → GAME_OVER"""
        if self._state == ClientState.PLAYING:
            self._state = ClientState.GAME_OVER
            
    def on_disconnected(self):
        """Transition: ANY → DISCONNECTED"""
        self._state = ClientState.DISCONNECTED
        self._player_color = None
        self._session_id = None
    
    # State queries    
    def can_join(self) -> bool:
        """Can join game?"""
        return self._state == ClientState.CONNECTED
        
    def can_start(self) -> bool:
        """Can start game?"""
        return self._state == ClientState.JOINED
        
    def can_move(self) -> bool:
        """Can make moves?"""
        return self._state == ClientState.PLAYING
        
    def can_display_board(self) -> bool:
        """Can display board?"""
        return self._state in (ClientState.PLAYING, ClientState.GAME_OVER)
    
    def get_state_name(self) -> str:
        """Get readable state name"""
        return self._state.name