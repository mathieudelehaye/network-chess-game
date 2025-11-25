"""Timer utilities for chess game timing.

Provides timestamp formatting functions.
"""


def format_timestamp(seconds: int) -> str:
    """
    Format seconds into HH:MM:SS format.

    Args:
        seconds: Total seconds elapsed since game start

    Returns:
        Formatted time string in HH:MM:SS format
    """
    hours = seconds // 3600
    minutes = (seconds % 3600) // 60
    secs = seconds % 60

    return f"{hours:02d}:{minutes:02d}:{secs:02d}"
