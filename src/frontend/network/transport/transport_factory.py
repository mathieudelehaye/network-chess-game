from network.transport.transport_interface import TransportMode
from network.transport.tcp.tcp_transport import TcpTransport
from network.transport.ipc.ipc_transport import IpcTransport
from network.transport.transport_interface import ITransport


class TransportFactory:
    """Factory for creating transport instances"""

    @staticmethod
    def create(fd: int, mode: TransportMode) -> ITransport:
        """
        Create a transport instance based on the network mode.

        @param fd File descriptor of the connected socket
        @param mode Network mode (TCP or IPC)
        @return Transport instance
        """
        if mode == TransportMode.IPC:
            return IpcTransport(fd)
        elif mode == TransportMode.TCP:
            return TcpTransport(fd)
        else:
            raise ValueError(f"Unsupported network mode: {mode}")