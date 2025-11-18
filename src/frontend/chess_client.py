#!/usr/bin/env python3
"""Chess client for sucden-fin-chess server."""

import argparse
import signal
import sys
import time
from pathlib import Path
from network.client import Client
from network.network_mode import NetworkMode
from utils.logger import Logger


def parse_arguments():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="Chess client for sucden-fin-chess server",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  chess_client                                    # Interactive mode
  chess_client -f ./test/game/game_01             # Play from file
  chess_client -i 192.168.1.100 -p 3000           # Custom server
  chess_client -v -f ./test/game/game_01          # Verbose mode
        """
    )
    
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose/debug logging"
    )
    
    parser.add_argument(
        "-i", "--ip",
        default="127.0.0.1",
        help="Server IP address (default: 127.0.0.1)"
    )
    
    parser.add_argument(
        "-p", "--port",
        type=int,
        default=2000,
        help="Server port (default: 2000)"
    )
    
    parser.add_argument(
        "-f", "--file",
        type=Path,
        help="Game file to play (non-interactive mode)"
    )
    
    return parser.parse_args()


def interactive_mode(client: Client, logger: Logger):
    """Run client in interactive mode."""
    logger.info("Commands:")
    logger.info("  - Type moves (e.g., 'e2-e4')")
    logger.info("  - Type 'display_board' to see current position")
    logger.info("  - Type 'quit' to exit")

    try:
        while True:
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
                    time.sleep(0.1)
                continue

            # Otherwise treat as move
            move_msg = {"move": user_input}
            if client.send_message(move_msg):
                time.sleep(0.1)

    except (KeyboardInterrupt, EOFError):
        logger.info("\nExiting...")


def file_mode(client: Client, logger: Logger, filepath: Path, chunk_size: int = 4096):
    """
    Send a game file to server in chunks.
    
    Args:
        client: Client instance
        logger: Logger instance
        filepath: Path to game file
        chunk_size: Size of each chunk in bytes
    
    Returns:
        True if upload successful
    """
    if not filepath.exists():
        logger.error(f"File not found: {filepath}")
        return False
    
    try:
        file_size = filepath.stat().st_size
        filename = filepath.name
        
        # Calculate total chunks
        chunks_total = (file_size + chunk_size - 1) // chunk_size
        
        logger.info(f"Uploading {filename} ({file_size} bytes, {chunks_total} chunks)")
        
        with open(filepath, 'r', encoding='utf-8') as f:
            chunk_current = 0
            
            while True:
                # Read chunk
                chunk_data = f.read(chunk_size)
                if not chunk_data:
                    break
                
                chunk_current += 1
                
                # Build message
                message = {
                    "command": "upload_game",
                    "metadata": {
                        "filename": filename,
                        "total_size": file_size,
                        "chunks_total": chunks_total,
                        "chunk_current": chunk_current
                    },
                    "data": chunk_data
                }
                
                # Send chunk
                if not client.send_message(message):
                    logger.error(f"Failed to send chunk {chunk_current}/{chunks_total}")
                    return False
                
                # Log progress
                percent = (chunk_current * 100) // chunks_total
                if chunk_current % 10 == 0 or chunk_current == chunks_total:
                    logger.info(f"Upload progress: {percent}% ({chunk_current}/{chunks_total})")
                
                # Small delay to avoid overwhelming server
                time.sleep(0.01)
        
        logger.info(f"Upload complete: {filename}")
        logger.info("Waiting for server to process game...")
        
        # Wait for server to process and send back results
        time.sleep(2.0)
        
        return True
        
    except Exception as e:
        logger.error(f"Error uploading file: {e}")
        return False


def main():
    args = parse_arguments()
    logger = Logger()

    # Set log level based on verbose flag
    if args.verbose:
        logger.set_level("DEBUG")

    # Create client
    client = Client(mode=NetworkMode.TCP, host=args.ip, port=args.port)

    # Handle Ctrl+C gracefully
    def signal_handler(sig, frame):
        logger.info("\nDisconnecting...")
        client.disconnect()
        sys.exit(0)

    signal.signal(signal.SIGINT, signal_handler)

    # Connect to server
    logger.info(f"Connecting to {args.ip}:{args.port}...")
    if not client.connect():
        logger.error("Failed to connect to server")
        logger.info(f"Make sure the server is running on {args.ip}:{args.port}")
        return 1

    logger.info("Connected to server")

    # Choose mode based on arguments
    if args.file:
        # File mode (non-interactive) - now uses chunking
        success = file_mode(client, logger, args.file)
        client.disconnect()
        return 0 if success else 1
    else:
        # Interactive mode
        interactive_mode(client, logger)
        client.disconnect()
        return 0


if __name__ == "__main__":
    sys.exit(main())