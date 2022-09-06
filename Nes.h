//
// Created by miguelcoelho on 11-07-2022.
//

#ifndef NES_H
#define NES_H

#include <cstdint>
#include <random>
#include "Rom.h"

using namespace std;

class Nes {
public:
    Nes();


    uint16_t pc{};
    uint16_t stack[256]{}; // 255?
    uint8_t memory[65536]{};
    uint8_t sp{};
    uint8_t reg_A;
    uint8_t reg_X;
    uint8_t reg_Y;
    uint8_t reg_stat;
    Rom rom;

    uint8_t getRandomByte();
    default_random_engine randGen;
    uniform_int_distribution<uint8_t> randByte;

    void LoadROM(char const *filename);
    uint8_t *getROM();

    void setCarryFlag(uint8_t val);

    void setFlag(uint8_t flag_idx, uint8_t val);

    void setZeroFlag(uint8_t val);

    void setInterruptFlag(uint8_t val);

    void setDecimalModeFlag(uint8_t val);

    void setBreakFlag(uint8_t val);

    void setOverflowFlag(uint8_t val);

    void setNegativeFlag(uint8_t val);

    int getFlag(uint8_t flag_idx);

    int getCarryFlag();

    int getZeroFlag();

    int getInterruptFlag();

    int getDecimalModeFlag();

    int getBreakFlag();

    int getOverflowFlag();

    int getNegativeFlag();

    static int getBit(uint8_t bit_index, uint8_t value);
};


#endif //NES_H
