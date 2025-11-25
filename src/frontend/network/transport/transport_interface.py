"""Transport layer interface and mode enumeration.

Defines abstract transport interface using Strategy pattern
and transport mode enumeration for TCP/IPC selection.
"""

from abc import ABC, abstractmethod
from enum import Enum


class TransportMode(Enum):
    """Transport protocol selection for network communication."""

    TCP = "tcp"
    IPC = "ipc"


class ITransport(ABC):
    """Transport layer interface (Strategy pattern).

    Defines abstract methods for transport implementations.
    """

    @abstractmethod
    def start(self, host: str, port: int) -> bool:
        """Start receiving data on the transport.

        Args:
            host: Server hostname or IP address
            port: Server port number

        Returns:
            bool: True if started successfully
        """
        pass

    @abstractmethod
    def send(self, data: bytes) -> bool:
        """Send data over the transport.

        Args:
            data: Bytes to send

        Returns:
            bool: True if sent successfully
        """
        pass

    @abstractmethod
    def close(self) -> None:
        """Close the transport connection."""
        pass
