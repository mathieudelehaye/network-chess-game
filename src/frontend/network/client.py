import socket
import threading
from typing import Optional
from network import NetworkMode
from network.transport_factory import TransportFactory
from session.client_session import ClientSession
from utils.logger import Logger

class Client:
    """Client that connects to the chess server"""
    
    def __init__(self, mode: NetworkMode, host: str, port: int):
        self.mode = mode
        self.host = host
        self.port = port
        self.client_fd: Optional[int] = None
        self.session: Optional[ClientSession] = None
        self._logger = Logger()
    
    def connect(self) -> bool:
        """Connect to server and create session"""
        try:
            # Create socket based on transport mode
            if self.mode == NetworkMode.ICP:
                self.client_fd = self._connect_unix()
            else:  # NetworkMode.TCP
                self.client_fd = self._connect_tcp()
            
            if self.client_fd < 0:
                return False
            
            self._logger.info(f"Connected to server, fd={self.client_fd}")
            
            # Create transport using factory
            transport = TransportFactory.create(self.client_fd, self.mode)
            
            # Create session to own the transport
            self.session = ClientSession(transport)
            
            # Start the session
            self.session.start()
            
            return True
        
        except Exception as e:
            self._logger.error(f"Connection failed: {e}")
            return False
    
    def _connect_tcp(self) -> int:
        """Create TCP socket and connect"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.connect((self.host, self.port))
            return sock.fileno()
        except Exception as e:
            self._logger.error(f"TCP connection failed: {e}")
            return -1
    
    def _connect_ipc() -> int:
        """Create IPC and connect"""
        # Implement 
        return -1
    
    def disconnect(self):
        if self.session:
            self.session.close()
        
        if self.client_fd >= 0:
            try:
                sock = socket.socket(fileno=self.client_fd)
                sock.shutdown(socket.SHUT_RDWR)
                sock.close()
            except:
                pass
            finally:
                self.client_fd = -1
        
        self._logger.info("Disconnected from server")
    
    def send_message(self, message) -> bool:
        """Send message through session"""
        if self.session:
            return self.session.send_message(message)
        return False