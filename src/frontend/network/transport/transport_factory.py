"""Transport factory for creating transport instances.

Factory pattern implementation for transport layer creation.
"""

from network.transport.transport_interface import TransportMode
from network.transport.tcp_transport import TcpTransport
from network.transport.ipc_transport import IpcTransport
from network.transport.transport_interface import ITransport


class TransportFactory:
    """Factory for creating transport instances.
    
    Creates appropriate transport based on mode selection.
    """

    @staticmethod
    def create(fd: int, mode: TransportMode) -> ITransport:
        """Create transport instance based on network mode.
        
        Args:
            fd: File descriptor of connected socket
            mode: Network mode (TCP or IPC)
            
        Returns:
            ITransport: Transport instance (TcpTransport or IpcTransport)
            
        Raises:
            ValueError: If unsupported network mode provided
        """
        if mode == TransportMode.IPC:
            return IpcTransport(fd)
        elif mode == TransportMode.TCP:
            return TcpTransport(fd)
        else:
            raise ValueError(f"Unsupported network mode: {mode}")