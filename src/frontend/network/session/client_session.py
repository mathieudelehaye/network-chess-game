"""Client session management.

Handles message framing, buffering, JSON parsing, and routing.
"""

import json
import socket
import threading
from typing import Optional, Callable
from controllers.response_router import ResponseRouter
from network.transport.transport_interface import ITransport
from utils.logger import Logger

class ClientSession:
    """Client session that owns a transport.
    
    Handles message framing, buffering, and JSON parsing.
    Routes complete messages to response router.
    """

    def __init__(
        self,
        transport: ITransport,
        router: ResponseRouter,
        _socket: Optional[socket.socket] = None,
        _running: bool = False,
        _receive_thread: Optional[threading.Thread] = None,
        _message_handler: Optional[Callable] = None):
        """Construct client session.
        
        Args:
            transport: Transport layer to use (ownership transferred)
            router: Response router for message handling
            _socket: Internal use only
            _running: Internal use only
            _receive_thread: Internal use only
            _message_handler: Internal use only
        """
        self.transport = transport
        self.router = router
        self.logger_ = Logger()
        self._active = False

        # Message buffering
        self._buffer = ""
        self._buffer_lock = threading.Lock()

    def start(self) -> None:
        """Start the session.
        
        Spawns transport's receive thread and begins message processing.
        """
        self._active = True

        # Start transport with our receive callback
        self.transport.start(self._on_receive)

        self.logger_.debug("Client session started")

    def _on_receive(self, raw: str) -> None:
        """Handle received data from transport.
        
        Buffers incomplete messages and parses complete ones.
        
        Args:
            raw: Raw data received from transport
        """
        if not self._active:
            return

        with self._buffer_lock:
            # Accumulate data into buffer
            self._buffer += raw

            # Process all complete messages (delimited by '\n')
            while "\n" in self._buffer:
                # Extract one complete message
                pos = self._buffer.index("\n")
                message = self._buffer[:pos]
                self._buffer = self._buffer[pos + 1 :]

                # Parse and handle this complete message
                self._handle_message(message)

    def _handle_message(self, reponse: str) -> None:
        """Parse and handle complete application message.
        
        Args:
            reponse: Complete message string (typo kept for compatibility)
        """
        if not self._active:
            return
        
        self.router.route(reponse)

    def send(self, message: dict) -> bool:
        """Send JSON message.
        
        Args:
            message: Dictionary to send as JSON
            
        Returns:
            bool: True if sent successfully
        """
        if not self._active:
            self.logger_.warning("Cannot send - session not active")
            return False

        try:
            json_str = json.dumps(message) + "\n"
            self.transport.send(json_str)
            self.logger_.debug(f"Sent: {json_str.strip()}")
            return True
        except Exception as e:
            self.logger_.error(f"Failed to send message: {e}")
            return False

    def close(self) -> None:
        """
        Close the session and transport.
        """
        # Atomic check-and-set to prevent double-close
        if not self._active:
            return

        self._active = False

        # Close transport (stops reader thread)
        self.transport.close()

        self.logger_.debug("Client session closed")
