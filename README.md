# NES Emulator

A NES emulator I built to practice C++

<p float="left">
  <img width="400"  alt="image" src="https://github.com/user-attachments/assets/2a478e5c-adae-4332-9084-e74f6cdd25f2" />
  <img width="400"  alt="image" src="https://github.com/user-attachments/assets/a8acca70-03a6-4214-84e5-e993199585e7" />
</p>


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

### Key Mappings

| Keyboard    | NES Controller |
|-------------|----------------|
| X           | B              |
| Z           | A              |
| A           | SELECT         |
| S           | START          |
| ARROW_UP    | UP             |
| ARROW_DOWN  | DOWN           |
| ARROW_LEFT  | LEFT           |
| ARROW_RIGHT | RIGHT          |


| Keyboard    | DEBUG Function   |
|-------------|------------------|
| SPACE       | Run/Pause        |
| C           | Step Instruction |
| F           | Step Frame       |
| P           | Change Palette   |


## Current Implementation Status

- CPU: All instructions implemented
- Memory: All memory support implemented
- ROM Loading: Support for mappers 0, 1 and 3
- PPU: Mostly functional, still in WIP
- APU: Not implemented yet
- Controllers: Basic keyboard interaction


## Thanks

This emulator is based on [olcNES](https://github.com/OneLoneCoder/olcNES) and the [NESDev wiki](https://www.nesdev.org/wiki/NES_reference_guide)
