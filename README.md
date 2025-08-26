# NES Emulator

A NES emulator I built just to practice C++

## Pre-requisites
Only tested with:
* C++ 17
* CMake 3.31.6
* MinGW 11.0w64

Dependencies (bundled in repo):
* Dear ImGui v1.92.0
* SDL 2.32.6

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


## Thanks

This emulator is based on [olcNES](https://github.com/OneLoneCoder/olcNES) and the [NESDev wiki](https://www.nesdev.org/wiki/NES_reference_guide)