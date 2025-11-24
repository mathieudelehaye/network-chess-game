# Set executable name
set(EXE_NAME chess_server)

include(SetupDoxygen)
include(SetupChessLibrary)

# Find vcpkg dependency packages
find_package(nlohmann_json CONFIG REQUIRED)

# Automatically find source files
file(GLOB_RECURSE EXE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Add executable to build
add_executable(${EXE_NAME} 
    ${EXE_SOURCES}
)

# Include project headers
target_include_directories(${EXE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}        
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/network
    ${CMAKE_CURRENT_SOURCE_DIR}/network/session
    ${CMAKE_CURRENT_SOURCE_DIR}/network/transport
    ${CMAKE_CURRENT_SOURCE_DIR}/network/transport/ipc
    ${CMAKE_CURRENT_SOURCE_DIR}/network/transport/tcp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Link libraries
target_link_libraries(${EXE_NAME} PRIVATE 
    spdlog::spdlog
    nlohmann_json::nlohmann_json
    chess-library::chess-library
    chess_parser   # Our parser library module
)

# Set optimization flags for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(${EXE_NAME} PRIVATE -O3 -march=native)
endif()

# Enable warnings
target_compile_options(${EXE_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Copy built executable to bin/backend
add_custom_command(
    TARGET ${EXE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_SOURCE_DIR}/../../bin/backend
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${EXE_NAME}>
            ${CMAKE_SOURCE_DIR}/../../bin/backend
)