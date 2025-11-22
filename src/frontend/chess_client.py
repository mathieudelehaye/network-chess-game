#!/usr/bin/env python3
"""Chess client for sucden-fin-chess server."""

import argparse
import sys
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
  chess_client.py # Interactive multiplayer
  mode 
  chess_client.py -f ./test/game/game_01 # Play from file
  (single-player) 
  chess_client.py -i 192.168.1.100 -p 3000 # Custom server
  chess_client.py -v -f ./test/game/game_01 # Verbose mode
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
        default=None,
        help="Game file to play (non-interactive mode)"
    )
    
    return parser.parse_args()


def main():
    """Main entry point"""
    args = parse_arguments()
    logger = Logger()
    
    # Set log level
    if args.verbose:
        logger.set_level("DEBUG")
    
    try:
        logger.info("Starting chess client...")
        
        client = Client(
            mode=NetworkMode.TCP,
            host=args.ip,
            port=args.port,
            game_file=args.file
        )
        
        client.start()
        
        print("Press Enter to exit...")
        input()  # Wait for user to press any key
        
        logger.info("Stopping client...")
        client.stop()
    
    except KeyboardInterrupt:
        logger.info("\nReceived interrupt signal - shutting down...")
        # Gracefully close
        if 'client' in locals():
            client.stop()
        logger.info("Client shut down gracefully")
        return 0
        
    except RuntimeError as e:
        logger.critical(f"Client initialisation failed: {e}")
        return 1
    except Exception as e:
        logger.critical(f"Unexpected error: {e}")
        return 2
    
    return 0


if __name__ == "__main__":
    sys.exit(main())