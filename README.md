# sucden-fin-chess
Technical interview for Sucden Financial

# Or run directly with verbose output
./test/chess_tests --gtest_filter=*Parser*

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
./bin/backend/chess_server

# Run unit tests
ctest --test-dir build/backend/debug/test --output-on-failure
```

- To generate the documentation (not auto-generated on every build):
```
cd /path/to/project/root

cmake --build build/backend/debug/ --target docs
```

## Frontend

```
cd /path/to/project/root/src/frontend

env="/path/to/project/root/.venv"
uv venv $env --python python3.11
source "$env/bin/activate"
python --version && which python

uv pip install -r requirements.txt

# Run client instance
python -m chess_client

# Run unit tests
pytest test/network/test_tcp_transport.py -v

deactivate
```

- To generate the documentation (not auto-generated on every build):
```
cd /path/to/project/root/doc/frontend

rm -rf html

sphinx-build -b html . html
```