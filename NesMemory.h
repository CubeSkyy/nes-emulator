//
// Created by miguelcoelho on 29-06-2023.
//

#ifndef NES_EMULATOR_NESMEMORY_H
#define NES_EMULATOR_NESMEMORY_H

#define RAM_SIZE 0x10000

#include <cstdint>
#include <vector>

using namespace std;

class NesMemory {
public:
    NesMemory();

    uint8_t getByte(uint16_t addr);
    uint16_t  getWord(uint16_t addr);

    void setByte(uint16_t addr, uint8_t val);
    void setWord(uint16_t addr, uint16_t val);

    void reset();
private:
    vector<uint8_t> _ram;
};

#endif //NES_EMULATOR_NESMEMORY_H
