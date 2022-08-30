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

    const unsigned int START_ADDRESS = 0x200;

    void LoadROM(char const *filename);

    uint8_t getRandomByte();

    // Instruction set
    void OP_00E0();


    default_random_engine randGen;
    uniform_int_distribution<uint8_t> randByte;
};


#endif //NES_H
