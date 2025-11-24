# Parser library (shared between exe and tests)

# Set library name
set(LIB_NAME chess_parser)

# Generate parsers from grammars
antlr_target(SimpleChessGameParser 
    ${CMAKE_CURRENT_SOURCE_DIR}/SimpleNotation/SimpleChessGame.g4
    PACKAGE chess
    VISITOR
)

antlr_target(PGNGameParser 
    ${CMAKE_CURRENT_SOURCE_DIR}/PGN/PGN.g4
    PACKAGE chess
    VISITOR
)

# Automatically find source files
file(GLOB_RECURSE EXE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Create parser library
add_library(${LIB_NAME} STATIC
    ${EXE_SOURCES}
    ${CMAKE_SOURCE_DIR}/exe/utils/Logger.cpp
    ${ANTLR_SimpleChessGameParser_CXX_OUTPUTS}
    ${ANTLR_PGNGameParser_CXX_OUTPUTS}
)

# Export ANTLR output directories for other targets
set(ANTLR_SimpleChessGameParser_OUTPUT_DIR 
    "${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated_src/SimpleChessGame" 
    PARENT_SCOPE)
set(ANTLR_PGNGameParser_OUTPUT_DIR 
    "${CMAKE_CURRENT_BINARY_DIR}/antlr4cpp_generated_src/PGN" 
    PARENT_SCOPE)

# Include directories for this library
target_include_directories(${LIB_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/parser
    ${CMAKE_SOURCE_DIR}/parser/PGN
    ${CMAKE_SOURCE_DIR}/parser/SimpleNotation
    ${CMAKE_SOURCE_DIR}/exe/utils
    ${ANTLR_SimpleChessGameParser_OUTPUT_DIR}
    ${ANTLR_PGNGameParser_OUTPUT_DIR}
)

target_include_directories(${LIB_NAME} SYSTEM PUBLIC
    "${ANTLR_SOURCE_DIR}/runtime/Cpp/runtime/src"
)

target_link_libraries(${LIB_NAME} PUBLIC
    antlr4_shared
    spdlog::spdlog
)