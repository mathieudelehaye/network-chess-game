import socket
from typing import Optional
from network.network_mode import NetworkMode
from network.transport.transport_factory import TransportFactory
from network.session.client_session import ClientSession
from utils.logger import Logger


class Client:
    """Client that connects to the chess server"""

    def __init__(self, mode: NetworkMode, host: str, port: int):
        self.mode = mode
        self.host = host
        self.port = port
        self.socket: Optional[socket.socket] = None
        self.session: Optional[ClientSession] = None
        self._logger = Logger()

    def connect(self) -> bool:
        """Connect to server and create session"""
        try:
            self._logger.info("Trying to connect to server...")

            # Create socket based on transport mode
            if self.mode == NetworkMode.IPC:
                # to implement
                pass
            else:  # NetworkMode.TCP
                self.socket = self._connect_tcp()

            fd = self.socket.fileno()

            if fd < 0:
                self._logger.error(f"Incorrect file descriptor: {fd}")
                return False

            # Create transport using factory
            transport = TransportFactory.create(fd, self.mode)

            # Create session to own the transport
            self.session = ClientSession(transport)

            # Start the session
            self.session.start()

            return True

        except (OSError, socket.error, ConnectionRefusedError, TimeoutError) as e:
            # Catch network-related exceptions
            self._logger.error(f"Connection failed: {e}")
            return False
        except ValueError as e:
            # Catch value validation errors
            self._logger.error(f"Invalid configuration: {e}")
            return False

    def _connect_tcp(self) -> int:
        """Create TCP socket and connect"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self.host, self.port))
            return sock
        except OSError as e:
            self._logger.error(f"TCP connection failed: {e}")
            return -1

    def _connect_ipc(self) -> int:
        """Create IPC and connect"""
        # Implement
        return -1

    def disconnect(self):
        if self.session:
            self.session.close()

        fd = self.socket.fileno()
        if fd >= 0:
            try:
                sock = socket.socket(fileno=fd)
                sock.shutdown(socket.SHUT_RDWR)
                sock.close()
            except OSError:
                # Socket might be already closed or invalid
                pass
            finally:
                self.socket = None

        self._logger.info("Disconnected from server")

    def send_message(self, message) -> bool:
        """Send message through session"""
        if not self.session:
            self._logger.error("Not connected to server")
            return False

        return self.session.send_message(message)
