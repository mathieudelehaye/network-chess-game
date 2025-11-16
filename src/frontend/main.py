import sys
import signal
from network.client import Client
from network.network_mode import NetworkMode
from utils.logger import Logger


def main():
    logger = Logger()
    
    # Create client
    client = Client(
        mode=NetworkMode.TCP,
        host="127.0.0.1",
        port=2000
    )
    
    # Handle Ctrl+C gracefully
    def signal_handler(sig, frame):
        print("\nDisconnecting...")
        client.disconnect()
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    
    # Connect to server
    if not client.connect():
        logger.error("Failed to connect to server")
        return 1
    
    print("Client connected to server. Press Ctrl+C to disconnect.")
    
    # Keep running until user interrupts
    try:
        input()  # Wait for Enter (like std::cin.get())
    except KeyboardInterrupt:
        pass
    
    # Cleanup
    client.disconnect()
    
    return 0


if __name__ == "__main__":
    sys.exit(main())