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
            cls._instance._initialized = False
        return cls._instance
    
    def __init__(self):
        """Initialize logger object"""
        if self._initialized:
            return
        
        self._initialized = True

        # change this with one of the values: INFO, DEBUG, WARNING, ERROR
        logging_level = logging.DEBUG 
        
        # Setup logging
        self.logger = logging.getLogger("ChessClient")
        self.logger.setLevel(logging_level)
        
        # Console handler
        console_handler = logging.StreamHandler(sys.stdout)
        console_handler.setLevel(logging_level)
        
        # File handler
        log_dir = Path("../../../log")
        log_dir.mkdir(exist_ok=True)
        file_handler = logging.FileHandler(log_dir / "client.log")
        file_handler.setLevel(logging_level)
        
        # Formatter
        formatter = logging.Formatter(
            '[%(asctime)s] %(levelname)s: %(message)s',
            datefmt='%Y-%m-%d %H:%M:%S'
        )
        
        console_handler.setFormatter(formatter)
        file_handler.setFormatter(formatter)
        
        self.logger.addHandler(console_handler)
        self.logger.addHandler(file_handler)
    
    def debug(self, message: str):
        self.logger.debug(message)
    
    def info(self, message: str):
        self.logger.info(message)
    
    def warning(self, message: str):
        self.logger.warning(message)
    
    def error(self, message: str):
        self.logger.error(message)
    
    def critical(self, message: str):
        self.logger.critical(message)