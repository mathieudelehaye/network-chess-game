import socket
from typing import Optional
from controllers.game_controller import GameController
from network.network_mode import NetworkMode
from network.transport.transport_factory import TransportFactory
from network.session.client_session import ClientSession
from utils.logger import Logger


class Client:
    """
    Client that connects to the chess server
    """

    def __init__(
        self, 
        mode: NetworkMode, 
        host: str, 
        port: int, 
        game_file: Optional[str] = None
        ):

        self.mode = mode
        self.host = host
        self.port = port
        self.port = port
        self.game_file = game_file
        
        self._logger = Logger()
        self.socket: Optional[socket.socket] = None
        self.session: Optional[ClientSession] = None
        self.controller: Optional[GameController] = None

    def start(self):
        """
        Start client - create session and call controller, since the client
        leads interactions with the server. 

        """
        self._logger.info(f"Connecting to {self.port}:{self.port}")
        
        # Connect (create socket)
        if not self._connect():
            raise RuntimeError("Failed to connect to server")
        
        # Create transport
        fd = self.socket.fileno()
        transport = TransportFactory.create(fd, self.mode)

        self.controller = GameController()

        self.session = ClientSession(
            transport=transport,
            controller=self.controller
        )

        self.controller.set_session(self.session)

        # Start session to begin receive loop
        self.session.start()
        
        self._logger.info(f"Client started on {self.mode.value}")

        # Run appropriate mode based on file
        if self.game_file:
            self._logger.info(f"Running file mode: {self.game_file}")
            self.controller.run_file_mode(self.game_file)
        else:
            self._logger.info("Running interactive mode")
            self.controller.run_interactive_mode()

    def _connect(self) -> bool:
        """
        Connect to server and create socket
        """
        try:
            self._logger.info(f"Connecting to {self.host}:{self.port}...")

            if self.mode == NetworkMode.IPC:
                self.socket = self._connect_ipc()
            else:  # NetworkMode.TCP
                self.socket = self._connect_tcp()

            if not self.socket:
                self._logger.error("Failed to create socket")
                return False

            return True

        except (OSError, socket.error, ConnectionRefusedError, TimeoutError) as e:
            self._logger.error(f"Connection failed: {e}")
            return False

    def _connect_tcp(self) -> Optional[socket.socket]:
        """Create TCP socket and connect"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self.host, self.port))
            self._logger.debug(f"TCP socket connected: fd={sock.fileno()}")
            return sock
        except OSError as e:
            self._logger.error(f"TCP connection failed: {e}")
            return None

    def _connect_ipc(self) -> Optional[socket.socket]:
        """Create Unix domain socket and connect"""
        self._logger.error("IPC mode not yet implemented")
        return None

    def stop(self):
        """
        Stop client - close session and socket
        """
        if self.session:
            self.session.close()
            self.session = None

        if self.socket:
            try:
                self.socket.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            finally:
                try:
                    self.socket.close()
                except OSError:
                    pass
                self.socket = None

        self._logger.info("Client stopped")