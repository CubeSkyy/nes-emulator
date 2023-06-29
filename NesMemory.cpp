//
// Created by miguelcoelho on 29-06-2023.
//

#include "NesMemory.h"

using namespace std;


NesMemory::NesMemory() {
    _ram.reserve(RAM_SIZE);
}

uint8_t NesMemory::getByte(uint16_t addr) {
    return _ram[addr];
}

uint16_t NesMemory::getWord(uint16_t addr) {
    return getByte(addr) + (uint16_t(getByte(addr + 1)) << 8);
}

void NesMemory::setByte(uint16_t addr, uint8_t val) {
    _ram[addr] = val;
}

void NesMemory::setWord(uint16_t addr, uint16_t val) {
    setByte(addr, val & 0xFF);
    setByte(addr + 1, val >> 8);
}

void NesMemory::reset() {
    fill(_ram.begin(), _ram.end(), 0);
}




