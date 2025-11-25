#!/usr/bin/env python3
"""Chess client application entry point.

Provides command-line interface for connecting to chess server
using TCP or IPC transport with GUI or console views.
"""

import argparse
import sys
from pathlib import Path
from network.client import Client
from network.transport.transport_interface import TransportMode
from utils.logger import Logger, logging
from views.view_factory import ViewFactory, ViewMode
from views.shared_console_view import SharedConsoleView


def parse_arguments():
    """Parse command-line arguments for chess client.

    Returns:
        argparse.Namespace: Parsed arguments with transport mode, view mode,
            connection details, and optional game file path.
    """
    parser = argparse.ArgumentParser(
        description="Chess client for sucden-fin-chess server",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Console mode (default)
  chess_client.py                              # Interactive multiplayer
  chess_client.py -f ./test/game/game_01       # Play from file
  chess_client.py -i 192.168.1.100 -p 3000     # Custom server
  chess_client.py -v -f ./test/game/game_01    # Verbose mode
  
  # GUI mode
  chess_client.py -g                           # Interactive with GUI board
  chess_client.py -g -f ./test/game/game_01    # GUI with file playback
  chess_client.py -g -l                        # GUI with Unix socket
  
  # Local IPC
  chess_client.py -l                           # Unix socket mode
  chess_client.py -l -s /tmp/custom.sock       # Custom socket path
        """,
    )

    parser.add_argument(
        "-v", "--verbose", action="store_true", help="Enable verbose/debug logging"
    )

    parser.add_argument(
        "-i", "--ip", default="127.0.0.1", help="Server IP address (default: 127.0.0.1)"
    )

    parser.add_argument(
        "-s",
        "--socket",
        default="/tmp/chess_server.sock",
        help="Unix socket path (default: /tmp/chess_server.sock)",
    )

    parser.add_argument(
        "-l",
        "--local",
        action="store_true",
        help="Use Unix domain socket for local IPC instead of TCP",
    )

    parser.add_argument(
        "-p", "--port", type=int, default=2000, help="Server port (default: 2000)"
    )

    parser.add_argument(
        "-f",
        "--file",
        type=Path,
        default=None,
        help="Game file to play (non-interactive mode)",
    )

    parser.add_argument(
        "-g",
        "--gui",
        action="store_true",
        help="Use graphical user interface instead of console",
    )

    return parser.parse_args()


def main():
    """Main entry point for chess client application.

    Initializes views, creates client, and starts interactive or file-based game mode.
    Handles graceful shutdown on KeyboardInterrupt and exceptions.

    Returns:
        int: Exit code (0=success, 1=initialization failure, 2=unexpected error)
    """
    args = parse_arguments()

    # Create logger and set log level
    logger = Logger()

    if args.verbose:
        logger.set_level("DEBUG")
        logger.info(f"Log level set to DEBUG")

    try:
        # Determine connection mode
        transport_mode = TransportMode.IPC if args.local else TransportMode.TCP

        if transport_mode == TransportMode.IPC:
            logger.info(f"Starting chess client (Unix socket: {args.socket})...")
        else:
            logger.info(f"Starting chess client (TCP: {args.ip}:{args.port})...")

        # Determine view mode and create views
        view_mode = ViewMode.GUI if args.gui else ViewMode.NOGUI

        # Create game view (GUI or NoGUI console)
        game_view = ViewFactory.create(view_mode)

        # Create shared console view (always needed for menus/messages)
        console_view = SharedConsoleView()

        if view_mode == ViewMode.GUI:
            logger.info("GUI mode enabled for board display")
        else:
            logger.info("Console mode for all display")

        # Create and start client
        client = Client(
            transport_mode=transport_mode,
            view_mode=view_mode,
            host=args.ip,
            port=args.port,
            socket_path=args.socket,
            game_file=args.file,
            game_view=game_view,
            console_view=console_view,
        )

        client.start()

        logger.info("Client finished")

    except KeyboardInterrupt:
        logger.info("\nReceived interrupt signal - shutting down...")
        return 0

    except RuntimeError as e:
        logger.critical(f"Client initialisation failed: {e}")
        return 1

    except Exception as e:
        logger.critical(f"Unexpected error: {e}")
        import traceback

        traceback.print_exc()
        return 2

    finally:
        # Cleanup resources
        if client:
            logger.info("Stopping client...")
            client.stop()

        if game_view:
            game_view.cleanup()

        logger.info("Client shut down gracefully")

    return 0


if __name__ == "__main__":
    sys.exit(main())
