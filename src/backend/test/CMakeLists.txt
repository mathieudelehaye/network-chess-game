# Set test executable name
set(EXE_TEST_NAME chess_server_tests)

# Find Google Test package
find_package(GTest CONFIG REQUIRED)
enable_testing()

# Collect all test source files
file(GLOB_RECURSE EXE_TEST_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# Add executable to build
add_executable(${EXE_TEST_NAME} 
    ${EXE_TEST_SOURCES}
)

target_include_directories(${EXE_TEST_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/parser/PGN
    ${CMAKE_SOURCE_DIR}/parser/SimpleNotation
)

# Link libraries
target_link_libraries(${EXE_TEST_NAME} PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    chess_parser   # Our parser library module
)

# Enable warnings
target_compile_options(${EXE_TEST_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

# Discover tests
include(GoogleTest)
gtest_discover_tests(${EXE_TEST_NAME})

# Copy test executable to bin directory
add_custom_command(
    TARGET ${EXE_TEST_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_SOURCE_DIR}/../../bin/backend
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${EXE_TEST_NAME}>
            ${CMAKE_SOURCE_DIR}/../../bin/backend
)