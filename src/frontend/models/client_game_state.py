class ClientGameState:
    """Client-side game state model"""
    
    def __init__(self):
        self.status_message = "Connecting..."
        self.white_joined = False
        self.black_joined = False
        self.can_start = False
        self.in_game = False
        self.current_turn = None
        self.phase = 0
    
    def update(self, server_response: dict):
        """Update from server status response"""
        self.status_message = server_response.get("status_message", "Unknown")
        self.white_joined = server_response.get("white_joined", False)
        self.black_joined = server_response.get("black_joined", False)
        self.can_start = server_response.get("can_start", False)
        self.in_game = server_response.get("in_game", False)
        self.current_turn = server_response.get("current_turn")
        self.phase = server_response.get("phase", 0)