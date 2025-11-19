#!/usr/bin/env python3
"""Chess client for sucden-fin-chess server."""

import argparse
import sys
from pathlib import Path
from controllers.game_controller import GameController
from utils.logger import Logger


def parse_arguments():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="Chess client for sucden-fin-chess server",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  main.py                                    # Interactive multiplayer mode
  main.py -f ./test/game/game_01             # Play from file (single-player)
  main.py -i 192.168.1.100 -p 3000           # Custom server
  main.py -v -f ./test/game/game_01          # Verbose mode
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


def main():
    """Main entry point"""
    args = parse_arguments()
    logger = Logger()
    
    # Set log level
    if args.verbose:
        logger.set_level("DEBUG")
    
    # Create controller
    controller = GameController(host=args.ip, port=args.port, logger=logger)
    
    # Run in appropriate mode
    if args.file:
        # Single-player: upload and play file
        return controller.run_file_mode(args.file)
    else:
        # Multiplayer: interactive menu
        return controller.run_interactive_mode()


if __name__ == "__main__":
    sys.exit(main())