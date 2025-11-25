"""Logging utilities for chess client.

Provides singleton logger with console and file output.
"""

import logging
import sys
from pathlib import Path


class Logger:
    """Class used to log rutime information."""

    _instance = None

    def __new__(cls):
        """Ensure only one instance exists (Singleton)"""
        if cls._instance is None:
            cls._instance = super().__new__(cls)
            cls._instance._initialised = False
        return cls._instance

    def __init__(self):
        """Initialise logger object"""

        # Constructor seems to be callable more than once, even for a Singleton class object.
        # pylint: disable=access-member-before-definition
        if hasattr(self, "_initialised") and self._initialised:
            return

        self._initialised = True

        logging_level = logging.INFO

        # Setup logging. Supported logging levels are: INFO, DEBUG, WARNING, ERROR 
        self.logger = logging.getLogger("ChessClient")
        self.logger.setLevel(logging_level)

        # Console handler
        self.console_handler = logging.StreamHandler(sys.stdout)
        self.console_handler.setLevel(logging_level)

        # File handler
        current_file = Path(__file__)
        project_root = current_file.parent.parent.parent.parent
        log_dir = project_root / "log"

        log_dir.mkdir(exist_ok=True)

        # Log file is re-written each time the client is run
        self.file_handler = logging.FileHandler(log_dir / "client.log", mode="w")
        self.file_handler.setLevel(logging_level)

        # Formatter
        formatter = logging.Formatter(
            "[%(asctime)s] %(levelname)s: %(message)s", datefmt="%Y-%m-%d %H:%M:%S"
        )

        self.console_handler.setFormatter(formatter)
        self.file_handler.setFormatter(formatter)

        self.logger.addHandler(self.console_handler)
        self.logger.addHandler(self.file_handler)

    def set_level(self, logging_level):
        self.logger.setLevel(logging_level)
        self.console_handler.setLevel(logging_level)
        self.file_handler.setLevel(logging_level)

    def debug(self, message: str):
        """Log debug message.
        
        Args:
            message: Debug message to log
        """
        self.logger.debug(message)

    def info(self, message: str):
        """Log informational message.
        
        Args:
            message: Info message to log
        """
        self.logger.info(message)

    def warning(self, message: str):
        """Log warning message.
        
        Args:
            message: Warning message to log
        """
        self.logger.warning(message)

    def error(self, message: str):
        """Log error message.
        
        Args:
            message: Error message to log
        """
        self.logger.error(message)

    def critical(self, message: str):
        """Log critical message.
        
        Args:
            message: Critical message to log
        """
        self.logger.critical(message)
