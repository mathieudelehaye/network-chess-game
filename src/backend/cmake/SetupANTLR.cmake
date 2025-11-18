# Setup ANTLR4 scanner / parser generator:
# https://github.com/antlr/antlr4/discussions/4418

include(FetchContent)

set(ANTLR_BUILD_SHARED ON)
set(ANTLR_TAG 4.13.1)
set(ANTLR_BUILD_CPP_TESTS OFF)

# Persistent download directory
set(ANTLR_DOWNLOAD_DIR "${CMAKE_SOURCE_DIR}/../../download")
set(ANTLR_EXECUTABLE "${ANTLR_DOWNLOAD_DIR}/antlr-${ANTLR_TAG}-complete.jar")
set(ANTLR_CPP_SOURCE_DIR "${ANTLR_DOWNLOAD_DIR}/antlr4-${ANTLR_TAG}")

# Create download directory if it doens't exist
file(MAKE_DIRECTORY "${ANTLR_DOWNLOAD_DIR}")

# Download ANTLR JAR if not exists
if(NOT EXISTS "${ANTLR_EXECUTABLE}")
    message(STATUS "Downloading ANTLR ${ANTLR_TAG} JAR...")
    file(
        DOWNLOAD
        "https://www.antlr.org/download/antlr-${ANTLR_TAG}-complete.jar"
        "${ANTLR_EXECUTABLE}"
        SHOW_PROGRESS
    )
    message(STATUS "Downloaded ANTLR JAR to: ${ANTLR_EXECUTABLE}")
else()
    message(STATUS "Using existing ANTLR JAR: ${ANTLR_EXECUTABLE}")
endif()

# Download ANTLR C++ runtime to persistent location if not exists
if(NOT EXISTS "${ANTLR_CPP_SOURCE_DIR}/runtime/Cpp/cmake/FindANTLR.cmake")
    message(STATUS "Downloading ANTLR ${ANTLR_TAG} C++ runtime...")
    
    FetchContent_Declare(
        antlr
        GIT_REPOSITORY https://github.com/antlr/antlr4
        GIT_TAG ${ANTLR_TAG}
        GIT_SHALLOW    TRUE # only perform a shallow clone (clone with `--depth 1` option), so avoid downloading the whole commit history.
        SOURCE_DIR "${ANTLR_CPP_SOURCE_DIR}"
        SOURCE_SUBDIR "runtime/Cpp"
    )
    FetchContent_MakeAvailable(antlr)
    message(STATUS "ANTLR C++ runtime downloaded to: ${ANTLR_CPP_SOURCE_DIR}")
else()
    message(STATUS "Using existing ANTLR C++ runtime at: ${ANTLR_CPP_SOURCE_DIR}")
    
    # Manually add the ANTLR runtime library since we skipped FetchContent
    add_subdirectory(
        "${ANTLR_CPP_SOURCE_DIR}/runtime/Cpp"
        "${CMAKE_BINARY_DIR}/_deps/antlr-build"
        EXCLUDE_FROM_ALL
    )
endif()

# Cache the source directory for future CMake runs
set(ANTLR_SOURCE_DIR "${ANTLR_CPP_SOURCE_DIR}" CACHE PATH "ANTLR C++ source directory" FORCE)

# Always include FindANTLR.cmake to define antlr_target command
if(EXISTS "${ANTLR_SOURCE_DIR}/runtime/Cpp/cmake/FindANTLR.cmake")
    include(${ANTLR_SOURCE_DIR}/runtime/Cpp/cmake/FindANTLR.cmake)
    message(STATUS "antlr_target command available from: ${ANTLR_SOURCE_DIR}")
else()
    message(FATAL_ERROR "FindANTLR.cmake not found at: ${ANTLR_SOURCE_DIR}/runtime/Cpp/cmake/FindANTLR.cmake")
endif()