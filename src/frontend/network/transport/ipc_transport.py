import os
import socket
import threading
from typing import Callable, Optional

from network.transport.transport_interface import ITransport
from utils.logger import Logger


class IpcTransport(ITransport):
    """Class for Unix domain socket (IPC) transport"""

    def __init__(self, fd: int):
        """
        Construct an IPC transport with an existing socket file descriptor.

        @param fd The file descriptor of the connected Unix socket.
        """
        self.fd = fd
        self.running = False
        self.reader_thread: Optional[threading.Thread] = None
        self.logger_ = Logger()

    def __del__(self):
        """Destructor ensures that the socket is closed."""
        self.close()

    def start(self, on_receive: Callable[[str], None]) -> None:
        """
        Start receiving data on the transport.

        @param on_receive Callback function to handle received data.
        """
        if self.fd < 0:
            self.logger_.error("Cannot start: invalid file descriptor")
            return

        self.running = True
        self.logger_.debug(f"Starting reader thread for Unix socket fd {self.fd}")

        def reader_loop():
            self.logger_.debug(f"Reader thread started for Unix socket fd {self.fd}")

            while self.running:
                try:
                    # Read from socket (max 4096 bytes)
                    data = os.read(self.fd, 4096)

                    # Connection closed or broken
                    if len(data) == 0:
                        self.logger_.info("Server closed connection")
                        self.running = False
                        break

                    payload = data.decode("utf-8")
                    on_receive(payload)

                except OSError as e:
                    if self.running:
                        self.logger_.error(
                            f"Read error on Unix socket fd {self.fd}: {e}"
                        )
                    self.running = False
                    break
                except Exception as e:
                    self.logger_.error(
                        f"Unexpected read error on Unix socket fd {self.fd}: {e}"
                    )
                    self.running = False
                    break

            self.logger_.debug(f"Reader thread EXITING for Unix socket fd {self.fd}")

        self.reader_thread = threading.Thread(target=reader_loop, daemon=True)
        self.reader_thread.start()

    def send(self, data: str) -> None:
        """
        Send data over the transport.

        @param data The data to send.
        """
        if self.fd < 0:
            self.logger_.error("Cannot send: socket closed")
            return

        if not self.running:
            return

        try:
            os.write(self.fd, data.encode("utf-8"))
        except OSError as e:
            self.logger_.error(f"Write error on Unix socket fd {self.fd}: {e}")
            self.running = False
        except Exception as e:
            self.logger_.error(
                f"Unexpected send error on Unix socket fd {self.fd}: {e}"
            )
            self.running = False

    def close(self) -> None:
        """Close the Unix socket and terminate the reading loop."""
        if not self.running:
            return

        self.running = False
        self.logger_.debug(f"Closing Unix socket transport on fd {self.fd}")

        if self.fd >= 0:
            try:
                os.close(self.fd)
            except Exception:
                pass
            finally:
                self.fd = -1

        # Wait for reader thread to finish
        if self.reader_thread and self.reader_thread.is_alive():
            self.reader_thread.join(timeout=1.0)

        self.logger_.debug("Unix socket transport closed")
