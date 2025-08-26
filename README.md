# NES Emulator

A NES emulator written in C++17.

## Pre-requisites
Only tested with:
* C++ 17
* CMake 3.31.6
* MinGW 11.0w64

## Building

```bash
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

## Using the Emulator

To run a NES ROM:

```bash
.\NESGraphicsDebug.exe <rom.nes>
```

## Current Implementation Status

- CPU: All instructions implemented
- Memory: All memory support implemented
- ROM Loading: Support for mappers 0, 1 and 3
- PPU: Mostly functional, still in WIP
- APU: Not implemented yet
- Controllers: Basic keyboard interaction