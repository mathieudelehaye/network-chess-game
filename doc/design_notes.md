# sucden-fin-chess

Technical interview for Sucden Financial

# Design Notes

## Backend

We have implemented a full MVC architecture, including:

- **Controllers**: Transmit client messages from the network to the model in
  order to parse them and maintain the game state. The controllers serialise and
  deserialise application commands and responses into JSON, a lightweight and
  flexible format well suited for network communication.

- **Models**: Contain the `ChessGame` class, which is a thin wrapper around the
  `chess-library` third-party library
  (https://github.com/Disservin/chess-library). This library manages the game
  state and validates moves using both Simple Notation (i.e., based on source
  and destination squares like `e2-e4`) and PGN notation (supporting only basic
  commands). The models also include the `IGameState` abstract class (interface)
  to manage the game's finite state machine using the State design pattern.
  `GameContext` coordinates both these classes, and there should only be one
  instance of this class (and of the controller holding it), even when multiple
  clients are connected, since they all play the same game on the server
  instance.

- **Views**: No view layer has been implemented yet, but an improvement would be
  to use one to prepare JSON application responses for clients (currently
  handled by the controller).

- **Network**: A network stack implementing OSI layers 4 (transport) and 5
  (session) is available. The Factory and Strategy design patterns are used to
  create transport layers when a client connects via TCP or Unix socket.
  Decomposing the network stack into multiple layers and classes allows us to
  swap neatly between TCP and IPC/Unix socket protocols, since only the
  transport layer needs to be replaced. 

- **Utils**: Include a `Logger` class providing a unique instance for the whole
  backend application, implementing the Singleton design pattern.

The parser was generated using ANTLR (https://www.antlr.org), building a C++
parser from `.g4` grammar files. We provide grammar files for both Simple
Notation (e.g., `e2-e4`) and PGN notation. The parsing builds an Abstract Syntax
Tree (AST), which can then be traversed via the Visitor design pattern. Since we
wanted to test the parser in isolation, it is compiled as a static library so it
can be linked to both the backend and unit test applications.

The backend build is automated via CMake, for which we wrote different `.cmake`
scripts to simplify:
- The download, caching and setup of ANTLR (`SetupANTLR.cmake`)
- The download, caching and setup of the chess-library
  (`SetupChessLibrary.cmake`)
- The setup of Doxygen (`SetupDoxygen.cmake`)

Some unit tests are provided (for the parser) which leverage GoogleTest. We
believe unit testing is an important practice, but here we only provide a sample
of them in order to demonstrate our capability to integrate GoogleTest into the
C++ build toolchain.

## Frontend

An MVC architecture is also used, which contains:

- **Controllers**: Transmit commands to the server (`game_controller.py`) and
  process server responses (`response_router.py`), communicating with the
  client-side finite state machines hosted in the models.

- **Models**: Manage the game state client-side from responses sent by the
  server to the controllers. `client_context.py` handles state transitions,
  while `game_model.py` manages different state variables, such as which players
  have joined and whose turn it is.

- **Views**: Provide different viewports depending on the view mode (`gui` or
  `no_gui`). A Factory design pattern is used to create either a GUI view
  (`gui_view.py`) or a non-GUI view (`no_gui_console_view.py`). The latter means
  all interactions, including moves and player inputs, are managed via console
  text prompting. For GUI mode, we have integrated the `pygame` library
  (https://github.com/pygame/pygame), which allows entering commands by clicking
  the screen and displaying moves via sprites on a visual board. Many
  interactions are still managed via console prompting even in GUI mode.
  Therefore, we created a third viewport, a shared console
  (`shared_console_view.py`), which is used in both GUI and non-GUI modes.

- **Network**: As with the backend, implements layer 4 (transport: TCP or Unix
  socket) and layer 5 (session).

- **Utils**: Include a `Logger` class, also managed via the Singleton pattern
  (as with the backend), as well as utilities to format timestamps sent by the
  server.

The documentation can be generated with Sphinx.

Some unit tests are provided, leveraging `pytest` and `unittest.mock`, applied
to the TCP transport class to demonstrate the value of such tests. We don't
provide complete unit test coverage for the frontend application.

## Design Patterns Used

- **MVC (Model-View-Controller)**: Both backend and frontend
- **State Pattern**: Game state management (`IGameState`, `GameContext`)
- **Factory Pattern**: Transport layer creation, view creation
- **Strategy Pattern**: Transport protocol selection (TCP vs IPC)
- **Visitor Pattern**: AST traversal in ANTLR-generated parsers
- **Singleton Pattern**: Logger instances in both backend and frontend

## Key Technologies

**Backend:**
- C++20
- CMake build system
- ANTLR 4 for parser generation
- chess-library for game logic
- GoogleTest for unit testing
- Doxygen for documentation
- vcpkg for dependency management

**Frontend:**
- Python 3.11+
- pygame for GUI
- pytest for unit testing
- Sphinx for documentation