# Configure CMake and C++
cmake_minimum_required(VERSION 3.16)

# Give project a name
project(chess_game_backend LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to cmake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include ANTLR setup
# 
# TODO: use an external dependency manager like vcpkg,
# so cmake build dir can be deleted without 
# downloading the ANTLR tool again.
# But it requires more work: https://shybovycha.github.io/2022/11/07/configscript-parser.html#:~:text=ANTLR%20version%20incompatibility
# 
include(SetupANTLR)

find_package(spdlog CONFIG REQUIRED)

# Set output dir 
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # .a
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # .so
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)   # executables

# Add subdirectories
add_subdirectory(exe)
add_subdirectory(parser)
add_subdirectory(test)