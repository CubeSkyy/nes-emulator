# Collect all test source files
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Test executable
add_executable(NES_Tests ${TEST_SOURCES})
target_include_directories(NES_Tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(NES_Tests PRIVATE GTest::gtest_main GTest::gtest nes_core)

# Enable testing
include(GoogleTest)
gtest_discover_tests(NES_Tests)
