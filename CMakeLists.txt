cmake_minimum_required(VERSION 3.22)
project(nes_emulator)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(nes_emulator
        Nes.cpp
        Nes.h Rom.cpp Rom.h Instructions.cpp Instructions.h)
