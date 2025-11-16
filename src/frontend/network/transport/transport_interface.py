from abc import ABC, abstractmethod
from typing import Callable


class ITransport(ABC):
    """Transport layer interface (Strategy pattern)"""

    @abstractmethod
    def connect(self, host: str, port: int) -> bool:
        """Establish connection"""
        pass

    @abstractmethod
    def disconnect(self) -> None:
        """Close connection"""
        pass

    @abstractmethod
    def send(self, data: bytes) -> bool:
        """Send data"""
        pass

    @abstractmethod
    def receive(self, callback: Callable[[bytes], None]) -> None:
        """Start receiving data asynchronously"""
        pass

    @abstractmethod
    def is_connected(self) -> bool:
        """Check connection status"""
        pass
