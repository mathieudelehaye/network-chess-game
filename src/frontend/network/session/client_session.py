import json
import threading
from network.transport.transport_interface import ITransport

# from utils.json_parser import Message
from utils.logger import Logger


class ClientSession:
    """
    Client session that owns a transport.
    Handles message framing, buffering, and JSON parsing.
    """

    def __init__(self, transport: ITransport):
        """
        Construct a client session.

        @param transport The transport layer to use (ownership transferred)
        """
        self.transport = transport
        self._logger = Logger()
        # self._message_handler: Optional[Callable[[Message], None]] = None
        self._active = False

        # Message buffering
        self._buffer = ""
        self._buffer_lock = threading.Lock()

    # def start(self, message_handler: Optional[Callable[[Message], None]] = None) -> None:
    def start(self) -> None:
        """
        Start the session.
        Spawns the transport's receive thread.

        @param message_handler Callback for incoming messages
        """
        self._active = True

        # Start transport with our receive callback
        self.transport.start(self._on_receive)

        self._logger.debug("Client session started")

    def _on_receive(self, raw: str) -> None:
        """
        Handle received data from transport.
        Buffers incomplete messages and parses complete ones.

        @param raw Raw data received from transport
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
                message_str = self._buffer[:pos]
                self._buffer = self._buffer[pos + 1 :]

                # Parse and handle this complete message
                self._handle_message(message_str)

    def _handle_message(self, json_str: str) -> None:
        """
        Parse and handle a complete JSON message.

        @param json_str Complete JSON message string
        """
        if not self._active:
            return

        try:
            # Parse JSON response (fixed: was 'data', should be 'json_str')
            response = json.loads(json_str.strip())

            # Handle board display
            if "board" in response:
                print("\n" + response["board"])
                return

            # Handle strike data (validated move)
            if "strike_number" in response and "piece" in response:
                self._display_strike(response)
                return

            # Handle errors
            if "error" in response:
                self._logger.error(f"Server error: {response['error']}")
                if "expected" in response:
                    self._logger.info(f"Expected format: {response['expected']}")
                return

            # Generic response
            self._logger.info(f"Server response: {response}")

        except json.JSONDecodeError as e:
            self._logger.error(f"Invalid JSON from server: {e}")
            self._logger.debug(f"Raw data: {json_str}")

    def _display_strike(self, strike: dict) -> None:
        """Format and display strike information"""
        # Build human-readable message
        msg = f"{strike['strike_number']}. {strike['color']} {strike['piece']}"

        if strike.get("is_castling"):
            msg += f" does a {strike['castling_type']} castling"
            msg += f" from {strike['case_src']} to {strike['case_dest']}"
        elif strike.get("is_capture"):
            msg += f" on {strike['case_src']}"
            msg += f" takes {strike['captured_color']} {strike['captured_piece']}"
            msg += f" on {strike['case_dest']}"
        else:
            msg += f" moves from {strike['case_src']} to {strike['case_dest']}"

        # Add check/checkmate suffix
        if strike.get("is_checkmate"):
            msg += ". Checkmate"
        elif strike.get("is_check"):
            msg += ". Check"
        elif strike.get("is_stalemate"):
            msg += ". Stalemate"

        self._logger.info(msg)

    # def send_message(self, message: Message) -> bool:
    def send_message(self, message: dict) -> bool:
        """
        Send a JSON message.

        @param message Dictionary to send as JSON
        @return True if sent successfully
        """
        if not self._active:
            self._logger.warning("Cannot send - session not active")
            return False

        try:
            # Convert dict to JSON string (fixed: was message.to_json())
            json_str = json.dumps(message) + "\n"
            self.transport.send(json_str)
            self._logger.debug(f"Sent: {json_str.strip()}")
            return True
        except Exception as e:
            self._logger.error(f"Failed to send message: {e}")
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

        self._logger.debug("Client session closed")
