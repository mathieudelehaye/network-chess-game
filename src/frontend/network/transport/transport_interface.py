from abc import ABC, abstractmethod
from enum import Enum


class TransportMode(Enum):
    TCP = "tcp"
    IPC = "ipc"


class ITransport(ABC):
    """Transport layer interface (Strategy pattern)"""
    
    @abstractmethod
    def connect(self) -> bool:
        """
        Connect to the server.
        
        @return True if connection successful, False otherwise
        """
        pass

    @abstractmethod
    def start(self, host: str, port: int) -> bool:
        """Start receiving data on the transport"""
        pass

    @abstractmethod
    def send(self, data: bytes) -> bool:
        """Send data over the transport"""
        pass

    @abstractmethod
    def close(self) -> None:
        """Close the transport connection"""
        pass
