//
// Created by miguelcoelho on 06-09-2022.
//

#include "Instructions.h"
#include <functional>

using namespace std;

typedef std::map<std::string, std::function<void()>> functionMap;

Instructions::Instructions(Nes *nes_in) {
    nes = nes_in;
    loadFunctionMap();
}

void Instructions::loadFunctionMap() {
    functionMap_.emplace(0xA9, [this]() { OP_A9(); });
}


// Util functions
uint8_t Instructions::getNextCode() {
    return nes->getROM()[nes->pc + 0x04];
}

uint16_t Instructions::get16bitfrom8bit(uint8_t higher, uint8_t lower) {
    return (higher << 8 ) | (lower & 0xff);
}

uint16_t Instructions::getNext16Code() {
    return (nes->getROM()[nes->pc + 0x04] << 8 ) | (nes->getROM()[nes->pc + 0x08] & 0xff);
}

uint16_t Instructions::getNext16CodeOffset(uint8_t offset) {
    return (nes->getROM()[nes->pc + 0x04 + offset] << 8 ) | (nes->getROM()[nes->pc + 0x08  + offset] & 0xff);
}

// Addressing modes functions
uint8_t Instructions::getImmediate() {
    return getNextCode();
}

uint8_t Instructions::getZeroPage() {
    return nes->memory[getNextCode()];
}

uint8_t Instructions::getZeroPageX() {
    return nes->memory[getNextCode() + nes->reg_X];
}

uint8_t Instructions::getAbsolute() {
    return nes->memory[getNext16Code()];
}

uint8_t Instructions::getAbsoluteX() {
    return nes->memory[getNext16Code() + nes->reg_X];
}

uint8_t Instructions::getAbsoluteY() {
    return nes->memory[getNext16Code() + nes->reg_Y];
}

uint8_t Instructions::getIndirectY() {
    uint8_t low = nes->memory[getNextCode() + nes->reg_X];
    uint8_t high = nes->memory[getNextCode() + 0x04 + nes->reg_X];
    return nes->memory[get16bitfrom8bit(high, low)];
}

uint8_t Instructions::getIndirectX() {
    uint8_t low = nes->memory[getNextCode()];
    uint8_t high = nes->memory[getNextCode() + 0x04];
    return nes->memory[get16bitfrom8bit(high, low) + nes->reg_Y];
}
// Instructions

// LDA
void Instructions::set_LDA_flags() {
    if (nes->reg_A == 0) {
        nes->setCarryFlag(1);
    }
    if (Nes::getBit(6, nes->reg_A)) {
        nes->setNegativeFlag(1);
    }
}


// A9 LDA - Immediate
void Instructions::OP_A9() {
    nes->reg_A = getImmediate();
    set_LDA_flags();
}

// A5 LDA - Zero Page
void Instructions::OP_A5() {
    nes->reg_A = getZeroPage();
    set_LDA_flags();
}

// B5 LDA - Zero Page, X
void Instructions::OP_B5() {
    nes->reg_A = getZeroPageX();
    set_LDA_flags();
}

// AD LDA - Absolute
void Instructions::OP_AD() {
    nes->reg_A = getAbsolute();
    set_LDA_flags();
}

// BD LDA - Absolute, X
void Instructions::OP_BD() {
    nes->reg_A = getAbsoluteX();
    set_LDA_flags();
}

// B9 LDA - Absolute, Y
void Instructions::OP_B9() {
    nes->reg_A = getAbsoluteY();
    set_LDA_flags();
}

// A1 LDA - Indirect, X
void Instructions::OP_A1() {
    nes->reg_A = getIndirectX();
    set_LDA_flags();
}

// B1 LDA - Indirect, Y
void Instructions::OP_B1() {
    nes->reg_A = getIndirectY();
    set_LDA_flags();
}

// TODO: Do tests for each opcode