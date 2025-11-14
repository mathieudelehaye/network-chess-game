class ConsoleView:
    def interactive_loop(self, controller: GameController):
        """Read commands from stdin"""
        print("Interactive mode. Enter moves or commands (:help for info)")
        
        while True:
            try:
                line = input("> ").strip()
                
                # Skip empty lines
                if not line:
                    continue
                
                # Meta-commands (start with :)
                if line.startswith(':'):
                    self._handle_command(line, controller)
                
                # Everything else is assumed to be a move
                else:
                    controller.send_move(line)
                    
            except KeyboardInterrupt:
                controller.disconnect()
                break
    
    def _handle_command(self, cmd: str, controller: GameController):
        if cmd == ':quit' or cmd == ':q':
            controller.disconnect()
            sys.exit(0)
        elif cmd == ':watch':
            game_id = input("Enter game ID to watch: ")
            controller.request_watch(game_id)
        elif cmd.startswith(':upload'):
            # :upload path/to/file.pgn
            parts = cmd.split(maxsplit=1)
            if len(parts) == 2:
                controller.upload_script(parts[1])
            else:
                print("Usage: :upload <filepath>")
        elif cmd == ':help':
            self._show_help()