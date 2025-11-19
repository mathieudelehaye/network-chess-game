from network.transport.tcp.tcp_transport import TcpTransport
from network.transport.transport_interface import ITransport
from network.network_mode import NetworkMode


class TransportFactory:
    """Factory for creating transports - mirrors C++ TransportFactory"""

    @staticmethod
    def create(socket_fd: int, mode: NetworkMode) -> ITransport:
        """
        Create a transport from an existing socket file descriptor.

        @param socket_fd The connected socket file descriptor
        @param mode The network mode
        @return Transport instance
        """
        if mode == NetworkMode.TCP:
            return TcpTransport(socket_fd)
        elif mode == NetworkMode.IPC:
            # to implement
            return None
        else:
            raise ValueError(f"Unknown transport mode: {mode}")
