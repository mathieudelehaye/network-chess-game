# Setup chess-library (header-only library)
# https://github.com/Disservin/chess-library

set(CHESS_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/../../download/chess-library")

# Check if already downloaded
if(EXISTS ${CHESS_LIBRARY_DIR})
    message(STATUS "Using cached chess-library from: ${CHESS_LIBRARY_DIR}")
    
    # Create interface library
    add_library(chess-library INTERFACE)
    add_library(chess-library::chess-library ALIAS chess-library)
    
    target_include_directories(chess-library INTERFACE
        ${CHESS_LIBRARY_DIR}/include
    )
    
    message(STATUS "chess-library setup complete (cached)")
else()
    message(STATUS "Fetching chess-library from GitHub...")
    
    include(FetchContent)
    
    set(FETCHCONTENT_QUIET OFF)
    
    FetchContent_Declare(
        chess-library
        GIT_REPOSITORY https://github.com/Disservin/chess-library
        GIT_TAG master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        TIMEOUT 120  # Increased timeout for slow connections
    )
    
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(chess-library)
    
    if(NOT TARGET chess-library::chess-library)
        add_library(chess-library INTERFACE)
        add_library(chess-library::chess-library ALIAS chess-library)
        
        target_include_directories(chess-library INTERFACE
            ${chess-library_SOURCE_DIR}/include
        )
    endif()
    
    message(STATUS "chess-library setup complete (downloaded)")
endif()