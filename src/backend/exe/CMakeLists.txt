# Set executable name
set(EXE_NAME chess_server)

# Path to cmake scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Include Doxygen setup
include(SetupDoxygen)

# Include ANTLR setup
# 
# TODO: use an external dependency manager like vcpkg,
# so cmake build dir can be deleted without 
# downloading the ANTLR tool again.
# But it requires more work: https://shybovycha.github.io/2022/11/07/configscript-parser.html#:~:text=ANTLR%20version%20incompatibility
# 
include(SetupANTLR)

# Generate parser from grammar
antlr_target(SimpleChessGameParser 
    ${CMAKE_CURRENT_SOURCE_DIR}/models/parser/SimpleChessGame.g4
    PACKAGE chess
    VISITOR
)

# Find vcpkg dependency packages
find_package(spdlog CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Automatically find source files
file(GLOB_RECURSE EXE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Add executable to build
add_executable(${EXE_NAME} 
    ${EXE_SOURCES}
    ${ANTLR_SimpleChessGameParser_CXX_OUTPUTS}
)

# Include dependency headers - use SYSTEM to suppress warnings
target_include_directories(${EXE_NAME} SYSTEM PRIVATE 
    "${ANTLR_SOURCE_DIR}/runtime/Cpp/runtime/src"
)

# Include project headers
target_include_directories(${EXE_NAME} PRIVATE
    ${ANTLR_SimpleChessGameParser_OUTPUT_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}        
    ${CMAKE_CURRENT_SOURCE_DIR}/controllers
    ${CMAKE_CURRENT_SOURCE_DIR}/models
    ${CMAKE_CURRENT_SOURCE_DIR}/models/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/views
    ${CMAKE_CURRENT_SOURCE_DIR}/network
    ${CMAKE_CURRENT_SOURCE_DIR}/network/session
    ${CMAKE_CURRENT_SOURCE_DIR}/network/transport
    ${CMAKE_CURRENT_SOURCE_DIR}/network/transport/tcp
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# Link libraries
target_link_libraries(${EXE_NAME} PRIVATE 
    antlr4_shared
    spdlog::spdlog
    nlohmann_json::nlohmann_json
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