cmake_minimum_required(VERSION 3.22)
project(nes_emulator)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(nes_emulator
        Nes.cpp
        Nes.h Rom.cpp Rom.h Instructions.cpp NesMemory.cpp NesMemory.h)

add_subdirectory(Google_tests)