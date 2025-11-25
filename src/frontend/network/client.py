"""Chess client network connection management.

Handles connection setup, session management, and game mode selection.
"""

import socket
from pathlib import Path
from typing import Optional
from controllers.game_controller import GameController
from controllers.response_router import ResponseRouter
from network.transport.transport_interface import TransportMode
from network.transport.transport_factory import TransportFactory
from network.session.client_session import ClientSession
from utils.logger import Logger
from views.shared_console_view import SharedConsoleView
from views.view_factory import ViewMode
from views.view_interface import IView


class Client:
    """Chess client that connects to server.

    Manages connection lifecycle, session creation, and game mode execution.
    Supports both TCP and IPC transport with GUI or console views.
    """

    def __init__(
        self,
        transport_mode: TransportMode,
        view_mode: ViewMode,
        host: str = "localhost",
        port: int = 2000,
        socket_path: str = "/tmp/chess_server.sock",
        game_file: Optional[Path] = None,
        game_view: IView = None,
        console_view: SharedConsoleView = None,
    ):
        """Initialise chess client.

        Args:
            transport_mode: Network protocol (TCP or IPC)
            view_mode: Display mode (GUI or console)
            host: Server hostname (TCP mode)
            port: Server port (TCP mode)
            socket_path: Unix socket path (IPC mode)
            game_file: Optional game file for playback mode
            game_view: Game view instance
            console_view: Console view instance
        """

        self.transport_mode = transport_mode
        self.view_mode = view_mode
        self.host = host
        self.port = port
        self.socket_path = socket_path
        self.game_file = game_file
        self.game_view = game_view
        self.console_view = console_view

        self.logger_ = Logger()
        self.socket: Optional[socket.socket] = None
        self.session: Optional[ClientSession] = None
        self.controller: Optional[GameController] = None

    def start(self):
        """Start client and begin game.

        Creates session, controller, and runs appropriate game mode.
        Executes file-based playback if game_file provided, otherwise
        starts interactive mode.

        Raises:
            RuntimeError: If connection to server fails
        """
        if self.transport_mode == TransportMode.IPC:
            self.logger_.info(f"Connecting to Unix socket: {self.socket_path}")
        else:
            self.logger_.info(f"Connecting to {self.host}:{self.port}")

        # Connect (create socket)
        if not self._connect():
            raise RuntimeError("Failed to connect to server")

        # Create transport
        fd = self.socket.fileno()
        transport = TransportFactory.create(fd, self.transport_mode)

        self.controller = GameController(
            self.view_mode, self.game_view, self.console_view
        )
        self.router = ResponseRouter(self.controller, self.view_mode)

        self.session = ClientSession(transport=transport, router=self.router)

        self.controller.set_session(self.session)

        # Start session to begin receive loop
        self.session.start()

        self.logger_.info(f"Client started on {self.transport_mode.value}")

        # Run appropriate mode based on file
        if self.game_file:
            self.logger_.info(f"Running file mode: {self.game_file}")
            self.controller.run_file_mode(self.game_file)
        
        self.logger_.info("Running interactive mode")
        self.controller.run_interactive_mode()

    def _connect(self) -> bool:
        """Connect to server and create socket.

        Returns:
            bool: True if connection successful
        """
        try:
            if self.transport_mode == TransportMode.IPC:
                self.socket = self._connect_ipc()
            else:  # TransportMode.TCP
                self.socket = self._connect_tcp()

            if not self.socket:
                self.logger_.error("Failed to create socket")
                return False

            return True

        except (OSError, socket.error, ConnectionRefusedError, TimeoutError) as e:
            self.logger_.error(f"Connection failed: {e}")
            return False

    def _connect_tcp(self) -> Optional[socket.socket]:
        """Create TCP socket and connect.

        Returns:
            socket.socket: Connected socket or None on failure
        """
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self.host, self.port))
            self.logger_.debug(f"TCP socket connected: fd={sock.fileno()}")
            return sock
        except OSError as e:
            self.logger_.error(f"TCP connection failed: {e}")
            return None

    def _connect_ipc(self) -> Optional[socket.socket]:
        """Create Unix domain socket and connect.

        Returns:
            socket.socket: Connected socket or None on failure
        """
        try:
            sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            sock.connect(self.socket_path)
            self.logger_.info(
                f"Unix socket connected: {self.socket_path} (fd={sock.fileno()})"
            )
            return sock
        except OSError as e:
            self.logger_.error(f"Unix socket connection failed: {e}")
            return None

    def stop(self):
        """Stop client and close connections.

        Closes session and socket gracefully.
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

        conn_type = "Unix socket" if self.transport_mode == TransportMode.IPC else "TCP"
        self.logger_.info(f"Client stopped ({conn_type})")
