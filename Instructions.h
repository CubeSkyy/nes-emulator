//
// Created by miguelcoelho on 06-09-2022.
//

#ifndef NES_EMULATOR_INSTRUCTIONS_H
#define NES_EMULATOR_INSTRUCTIONS_H


#include "Nes.h"

#include <map>
#include <unordered_map>
#include <memory>
#include "string"
#include <functional>

class Instructions {


    Nes *nes;

    void loadFunctionMap();

    void OP_A9();

public:
    Instructions(Nes *nes);

    std::map<uint8_t, std::function<void()>> functionMap_;

    void OP_A5();

    void set_LDA_flags();

    void OP_B5();

    void OP_AD();

    void OP_BD();

    uint8_t getNextCode();

    uint16_t getNext16Code();

    void OP_B9();

    uint16_t getNext16CodeOffset(uint8_t);

    void OP_A1();

    void OP_B1();

    uint16_t get16bitfrom8bit(uint8_t lower, uint8_t higher);

    uint8_t getAbsolute();

    uint8_t getImmediate();

    uint8_t getZeroPage();

    uint8_t getZeroPageX();

    uint8_t getAbsoluteX();

    uint8_t getAbsoluteY();

    uint8_t getIndirectY();

    uint8_t getIndirectX();
};


#endif //NES_EMULATOR_INSTRUCTIONS_H
