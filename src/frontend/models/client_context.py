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
    Client-side FSM state management (Singleton).
    
    Manages client state transitions and current state data.
    Similar to Logger - ensures shared state across all controllers.
    """
     
    _instance = None

    def __new__(cls):
        """Ensure only one instance exists (Singleton)"""
        if cls._instance is None:
            cls._instance = super().__new__(cls)
        return cls._instance
    
    def __init__(self):
        self._state = ClientState.DISCONNECTED
        self._player_color: Optional[str] = None
        self._player_number = 0
        self._session_id: Optional[str] = None
        self._session_id: Optional[str] = None
        
    @property
    def state(self) -> ClientState:
        return self._state
        
    @property
    def player_color(self) -> Optional[str]:
        return self._player_color
    
    @property
    def player_number(self) -> Optional[int]:
        return self._player_number
        
    @property
    def session_id(self) -> Optional[str]:
        return self._session_id
    
    # State transitions
    def on_connected(self, session_id: str):
        """Transition: DISCONNECTED → CONNECTED"""
        if self._state == ClientState.DISCONNECTED:
            self._state = ClientState.CONNECTED
            self._session_id = session_id
            
    def on_joined(self, session_id: str, single_player: bool, color: str):
        """Transition: CONNECTED → JOINED or CONNECTED → PLAYING (single-player mode)"""
        if not (self._state == ClientState.CONNECTED and self._session_id == session_id):
            return
        
        if not single_player:
            # Detect that player joined the server for a 2 player game 
            self._state = ClientState.JOINED # Start game command needs to be sent
            self._player_color = color
            self._player_number = 2
        else:
            # Detect that player joined the server as single player (i.e.
            # playing both White and Black) 
            self._state = ClientState.PLAYING # No start game command required
            self._player_color = ""
            self._player_number = 1

    def on_game_started(self):
        """Transition: JOINED → PLAYING"""
        if self._state == ClientState.JOINED:
            self._state = ClientState.PLAYING
            
    def on_game_over(self):
        """Transition: PLAYING → GAME_OVER"""
        if self._state == ClientState.PLAYING:
            self._state = ClientState.GAME_OVER

    def on_reset(self):
        """Transition: PLAYING → GAME_OVER"""
        if (
            self._state == ClientState.JOINED or
            self._state == ClientState.PLAYING or
            self._state == ClientState.GAME_OVER
        ):
            self._state = ClientState.CONNECTED
            
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