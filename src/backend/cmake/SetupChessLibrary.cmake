# Setup chess-library (header-only library)
# https://github.com/Disservin/chess-library

# TODO: locally cache the library, rather than downloading it again each time
# the CMAKE `build/` directory is deleted. Since the download only takes a few
# second, we haven't cached it yet.

include(FetchContent)

message(STATUS "Fetching chess-library...")

FetchContent_Declare(
    chess-library
    GIT_REPOSITORY https://github.com/Disservin/chess-library.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE # only perform a shallow clone (clone with `--depth 1` option), so avoid downloading the whole commit history.
)

# Don't build tests/examples
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(chess-library)

# The library is header-only, so we just need the include directory
if(NOT TARGET chess-library::chess-library)
    add_library(chess-library INTERFACE)
    add_library(chess-library::chess-library ALIAS chess-library)
    
    target_include_directories(chess-library INTERFACE
        ${chess-library_SOURCE_DIR}/include
    )
endif()

message(STATUS "chess-library setup complete")