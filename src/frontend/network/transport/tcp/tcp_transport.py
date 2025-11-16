import os
import threading
from typing import Callable, Optional
from ..transport_interface import ITransport


class TcpTransport(ITransport):
    """TCP transport - mirrors C++ TcpTransport"""

    def __init__(self, socket_fd: int):
        """
        Construct a TCP transport with an existing socket.

        @param socket_fd The file descriptor of the connected socket.
        """
        self.fd = socket_fd
        self.running = False
        self.reader_thread: Optional[threading.Thread] = None

    def __del__(self):
        """Destructor ensures that the socket is closed."""
        self.close()

    def start(self, on_receive: Callable[[str], None]) -> None:
        """
        Start receiving data on the transport.

        @param on_receive Callback function to handle received data.
        """
        self.running = True

        def reader_loop():
            while self.running:
                try:
                    # Read from socket (max 1024 bytes like C++ version)
                    data = os.read(self.fd, 1024)

                    # Connection closed or broken
                    if len(data) == 0:
                        self.running = False
                        break

                    payload = data.decode("utf-8")
                    on_receive(payload)

                except Exception as e:
                    self._logger.info(f"Read error: {e}")
                    self.running = False
                    break

        self.reader_thread = threading.Thread(target=reader_loop, daemon=True)
        self.reader_thread.start()

    def send(self, data: str) -> None:
        """
        Send data over the transport.

        @param data The data to send.
        """
        try:
            os.write(self.fd, data.encode("utf-8"))
        except Exception as e:
            self._logger.info(f"Send error: {e}")

    def close(self) -> None:
        """Close the TCP socket and terminate the reading loop."""
        self.running = False

        if self.fd >= 0:
            try:
                # Shutdown both directions
                import socket

                sock = socket.socket(fileno=self.fd)
                sock.shutdown(socket.SHUT_RDWR)
                sock.close()
            except:
                pass
            finally:
                self.fd = -1

        # Wait for reader thread to finish
        if self.reader_thread and self.reader_thread.is_alive():
            self.reader_thread.join(timeout=1.0)
