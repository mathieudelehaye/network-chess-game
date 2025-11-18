"""
Client-side game model.
Handles data transformation and business logic.
"""


class GameModel:
    """
    Model layer for client-side game logic.
    Transforms server data into human-readable formats.
    """

    @staticmethod
    def build_move_description(strike: dict) -> str:
        """
        Build human-readable move description from strike data.

        Args:
            strike: Strike data from server containing:
                - X: int
                - color: str ("white" or "black")
                - piece: str ("pawn", "rook", etc.)
                - case_src: str (e.g., "e2")
                - case_dest: str (e.g., "e4")
                - is_capture: bool (optional)
                - captured_piece: str (optional)
                - captured_color: str (optional)
                - is_castling: bool (optional)
                - castling_type: str (optional, "big" or "little")

        Returns:
            Human-readable move description
            
        Examples:
            >>> GameModel.build_move_description({
            ...     "strike_number": 1,
            ...     "color": "white",
            ...     "piece": "pawn",
            ...     "case_src": "e2",
            ...     "case_dest": "e4"
            ... })
            '1. white pawn moves from e2 to e4'
            
            >>> GameModel.build_move_description({
            ...     "strike_number": 3,
            ...     "color": "white",
            ...     "piece": "knight",
            ...     "case_src": "f3",
            ...     "case_dest": "h4",
            ...     "is_capture": True,
            ...     "captured_piece": "queen",
            ...     "captured_color": "black"
            ... })
            '3. white knight on f3 takes black queen on h4'
        """
        msg = f"{strike['strike_number']}. {strike['color']} {strike['piece']}"

        if strike.get("is_castling"):
            msg += f" does a {strike['castling_type']} castling"
            msg += f" from {strike['case_src']} to {strike['case_dest']}"
        elif strike.get("is_capture"):
            msg += f" on {strike['case_src']}"
            msg += f" takes {strike['captured_color']} {strike['captured_piece']}"
            msg += f" on {strike['case_dest']}"
        else:
            msg += f" moves from {strike['case_src']} to {strike['case_dest']}"

        return msg