# sucden-fin-chess

Technical interview for Sucden Financial

# Build and run

## Backend

- To build and run the backend server:
```
cd /path/to/project/root

rm -rf build/backend/debug

cmake \
  -S src/backend \
  -B build/backend/debug \
  -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux

cmake --build build/backend/debug -j$(nproc)

# Run server
./bin/backend/chess_server -v

# Run unit tests
ctest --test-dir build/backend/debug/test --output-on-failure
```

- To generate the documentation (not auto-generated on every build):
```
cd /path/to/project/root

cmake --build build/backend/debug/ --target docs
```

- IPC (Unix socket) mode:
```
./bin/backend/chess_server --local
```

## Frontend

- To run a client instance (single - player mode):
```
cd /path/to/project/root/src/frontend

env="/path/to/project/root/.venv"
uv venv $env --python python3.11
source "$env/bin/activate"
python --version && which python

# Run client 
PYTHONPATH=src/frontend python -m chess_client

# Run client with a game file
PYTHONPATH=src/frontend python -m chess_client -f test/game/game_01

# Run unit tests
pytest src/frontend/test/network/test_tcp_transport.py -v

deactivate
```

- To generate the documentation:
```
cd /path/to/project/root/doc/frontend

# initialise .venv environment 

rm -rf html

sphinx-build -b html . html

# deactivate .venv environment
```

- To run clients with two players:
  - Run a first client and select "Join as White (Black) Player
  - Run a first client and select "Join as Black (White) Player
  - Any player can press "Start Game" to launch the game.

- GUI mode (only available in single - player mode for now):
```
PYTHONPATH=src/frontend python -m chess_client --gui
```

- IPC (Unix socket) mode:
```
PYTHONPATH=src/frontend python -m chess_client --local
```
> **Warning**
> The server must also be listening for IPC (Unix socket).

- PGN mode: 
  - No modification is required to the client in order to use PGN game commands
    (instead of simple notation commands), if the server is configured in PGN
    mode.

- Other commands are available when the game has started. They include:
  - `:r`: restart the game
  - `:f <file name>`: upload a game file (similary to the `-f` command line argument)
  - `:d`: display a pretty-printed chess board (text mode)