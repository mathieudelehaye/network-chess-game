import sys
import time
import signal
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

    logger.info("Client connected to server.")
    logger.info("Commands:")
    logger.info("  - Type moves (e.g., 'e2-e4')")
    logger.info("  - Type 'display_board' to see current position")
    logger.info("  - Type 'quit' to exit")

    # Interactive command loop
    try:
        while True:
            # Get input from user
            user_input = input("\n> ").strip()

            if not user_input:
                continue

            if user_input.lower() in ["quit", "exit", "q"]:
                logger.info("Exiting...")
                break

            # Handle display_board command
            if user_input.lower() == "display_board":
                cmd_msg = {"command": "display_board"}
                if client.send_message(cmd_msg):
                    logger.debug("Sent display_board command")
                    time.sleep(0.1)  # Give server time to respond
                else:
                    logger.error("Failed to send command")
                continue

            # Otherwise treat as move
            move_msg = {"move": user_input}
            if client.send_message(move_msg):
                logger.debug(f"Sent move: {user_input}")
                time.sleep(0.1)  # Give server time to respond
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