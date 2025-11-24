# sucden-fin-chess
Technical interview for Sucden Financial

# Or run directly with verbose output
./test/chess_tests --gtest_filter=*Parser*

# Build and run

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

ctest --test-dir build/backend/debug/test --output-on-failure

./bin/backend/chess_server
```

- To generate the documentation (not auto-generated on every build):
```
cmake --build build/backend/debug/ --target docs
```
