cmake_minimum_required(VERSION 3.31)
project(NESEmulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Threads REQUIRED)

# SDL2 setup
set(SDL2_ROOT "${CMAKE_SOURCE_DIR}/external/SDL2-2.32.6")
# Use 64-bit MinGW-w64 by default
set(SDL2_INCLUDE_DIR "${SDL2_ROOT}/x86_64-w64-mingw32/include/SDL2")
set(SDL2_LIB_DIR "${SDL2_ROOT}/x86_64-w64-mingw32/lib")

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

# Add ImGui backends include directory
include_directories(${CMAKE_SOURCE_DIR}/external/imgui/backends)

# Include directories
include_directories(external)

# ImGui setup
set(IMGUI_ROOT "${CMAKE_SOURCE_DIR}/external/imgui")
set(IMGUI_INCLUDE_DIR "${IMGUI_ROOT}")
include_directories(${IMGUI_INCLUDE_DIR})
add_subdirectory(external/imgui)

# Google Test setup
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/7e2c425db2c2e024b2807bfe6d386f4ff068d0d6.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(src)
add_subdirectory(test)

