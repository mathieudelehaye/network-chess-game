import threading
from typing import Callable, Optional
from transport.transport_interface import ITransport
from utils.json_parser import Message
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
        self._message_handler: Optional[Callable[[Message], None]] = None
        self._active = False
        
        # Message buffering
        self._buffer = ""
        self._buffer_lock = threading.Lock()
    
    def start(self, message_handler: Optional[Callable[[Message], None]] = None) -> None:
        """
        Start the session.
        Spawns the transport's receive thread.
        
        @param message_handler Callback for incoming messages
        """
        self._active = True
        self._message_handler = message_handler
        
        # Start transport with our receive callback
        self.transport.start(self._on_receive)
        
        self._logger.info("Client session started")
    
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
            while '\n' in self._buffer:
                # Extract one complete message
                pos = self._buffer.index('\n')
                message_str = self._buffer[:pos]
                self._buffer = self._buffer[pos + 1:]
                
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
            # Parse JSON message
            message = Message.from_json(json_str)
            
            # Dispatch to handler
            if message and self._message_handler:
                self._message_handler(message)
                
        except Exception as e:
            self._logger.error(f"Message handling error: {e}")
    
    def send_message(self, message: Message) -> bool:
        """
        Send a JSON message.
        
        @param message The message to send
        @return True if sent successfully
        """
        if not self._active:
            return False
        
        try:
            json_str = message.to_json() + '\n'
            self.transport.send(json_str)
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
        
        self._logger.info("Client session closed")