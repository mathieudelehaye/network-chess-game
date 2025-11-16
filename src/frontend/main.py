import signal
import sys
import time
from network.client import Client
from network.network_mode import NetworkMode
from utils.logger import Logger


def main():
    logger = Logger()

    # Create client
    client = Client(mode=NetworkMode.TCP, host="127.0.0.1", port=2000)

    # Handle Ctrl+C gracefully
    def signal_handler(sig, frame):
        logger.info("\nDisconnecting...")
        client.disconnect()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    # Connect to server
    if not client.connect():
        logger.error("Failed to connect to server")
        return 1

    logger.info("Client connected to server. Type moves (e.g., 'a2-a4') or 'quit' to exit.")

    # Interactive move input loop
    try:
        while True:
            # Get move from user
            move_input = input("\nEnter move: ").strip()

            if not move_input:
                continue

            if move_input.lower() in ["quit", "exit", "q"]:
                logger.info("Exiting...")
                break

            # Send move to server
            move_msg = {"move": move_input}
            if client.send_message(move_msg):
                logger.info(f"Sent move: {move_input}")
                # Give server time to respond
                time.sleep(0.5)
            else:
                logger.error("Failed to send move")

    except KeyboardInterrupt:
        logger.info("\nInterrupted by user")
    except EOFError:
        logger.info("\nEnd of input")

    # Cleanup
    client.disconnect()

    return 0


if __name__ == "__main__":
    sys.exit(main())
