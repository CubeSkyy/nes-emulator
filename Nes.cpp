//
// Created by miguelcoelho on 11-07-2022.
//

#include <iostream>
#include "Nes.h"
#include "Rom.h"
#include "fstream"

#include <chrono>
#include <random>
#include <cstdio>
#include <cstring>
#include <bitset>
#include <memory>
//#include "include/spdlog/spdlog.h"

using namespace std;

Nes::Nes() {
    //Init RNG
    unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
    randGen.seed(seed);
    randByte = uniform_int_distribution<uint8_t>(0, 255U);

    _ram = make_unique<NesMemory>();
    // Reset memory and registers to 0
    reset(false);
}

uint8_t Nes::getRandomByte() {
    return randByte(randGen);
}

vector<uint8_t> Nes::getROM() {
    return rom.chr_rom;
}

void Nes::setFlag(uint8_t flag_idx, uint8_t val) {
    reg_stat = setBit(reg_stat, flag_idx, val);
}

uint8_t Nes::setBit(uint8_t value, uint8_t bit_idx, uint8_t bit_val) {
    if (bit_val == 1) {
        value |= 1UL << bit_idx;
    } else if (bit_val == 0) {
        value &= ~(1UL << bit_idx);
    }
    return value;
}


// TODO: Put this in util class
int Nes::getBit(uint8_t bit_index, uint8_t value) {
    return (value >> bit_index) & 1U;
}

int Nes::getFlag(uint8_t flag_idx) {
    return getBit(flag_idx, reg_stat);
}

void Nes::setCarryFlag(uint8_t val) { setFlag(0, val); }

void Nes::setZeroFlag(uint8_t val) { setFlag(1, val); }

void Nes::setInterruptFlag(uint8_t val) { setFlag(2, val); }

void Nes::setDecimalModeFlag(uint8_t val) { setFlag(3, val); }

void Nes::setBreakFlag(uint8_t val) { setFlag(4, val); }

void Nes::setOverflowFlag(uint8_t val) { setFlag(6, val); }

void Nes::setNegativeFlag(uint8_t val) { setFlag(7, val); }

int Nes::getCarryFlag() { return getFlag(0); }

int Nes::getZeroFlag() { return getFlag(1); }

int Nes::getInterruptFlag() { return getFlag(2); }

int Nes::getDecimalModeFlag() { return getFlag(3); }

int Nes::getBreakFlag() { return getFlag(4); }

int Nes::getOverflowFlag() { return getFlag(6); }

int Nes::getNegativeFlag() { return getFlag(7); }

void Nes::pushToStack(uint8_t value) {
    stack[sp] = value;
    sp--;
}

uint8_t Nes::pullFromStack() {
    sp++;
    return stack[sp];
}

void Nes::reset(bool test) {
    pc = 0;
    memset(stack, 0, sizeof(stack));
    _ram->reset();
    sp = 0xFD;
    reg_A = 0;
    reg_X = 0;
    reg_Y = 0;
    reg_stat = 0;
    rom = Rom();
    setZeroFlag(0);
    setNegativeFlag(0);
    setCarryFlag(0);
    setBreakFlag(0);
    setOverflowFlag(0);
    setInterruptFlag(0);
    setDecimalModeFlag(0);
    if (test) { rom.LoadTestParams(); }
}

void Nes::loop() {

    while (true) {
        uint8_t opcode = getROM()[pc];

        switch (opcode) {
            DECODE_ALU_OP_CODE(ORA)
            DECODE_ALU_OP_CODE(AND)
            DECODE_ALU_OP_CODE(EOR)
            DECODE_ALU_OP_CODE(ADC) 
            DECODE_ALU_OP_CODE(STA)
            DECODE_ALU_OP_CODE(LDA)
            DECODE_ALU_OP_CODE(CMP)
            DECODE_ALU_OP_CODE(SBC)

            DECODE_RMW_OP_CODE(ASL);
            DECODE_RMW_OP_CODE(ROL);
            DECODE_RMW_OP_CODE(LSR);
            DECODE_RMW_OP_CODE(ROR);

            DECODE_OP_CODE_DIRECT(BIT, 0x24, zp);
            DECODE_OP_CODE_DIRECT(BIT, 0x2C, abs);

            DECODE_OP_CODE_DIRECT(BCC, 0x90, rel);
            DECODE_OP_CODE_DIRECT(BCS, 0xB0, rel);
            DECODE_OP_CODE_DIRECT(BEQ, 0xF0, rel);
            DECODE_OP_CODE_DIRECT(BMI, 0x30, rel);
            DECODE_OP_CODE_DIRECT(BNE, 0xD0, rel);
            DECODE_OP_CODE_DIRECT(BPL, 0x10, rel);
            DECODE_OP_CODE_DIRECT(BVC, 0x50, rel);
            DECODE_OP_CODE_DIRECT(BVS, 0x70, rel);

            DECODE_OP_CODE_DIRECT(JMP, 0x4C, abs_jmp);
            DECODE_OP_CODE_DIRECT(JMP, 0x6C, ind_jmp);

            DECODE_OP_CODE_DIRECT(BRK, 0x00, imp);


            DECODE_OP_CODE_DIRECT(CLC, 0x18, imp);
            DECODE_OP_CODE_DIRECT(CLD, 0xD8, imp);
            DECODE_OP_CODE_DIRECT(CLI, 0x58, imp);
            DECODE_OP_CODE_DIRECT(CLV, 0xB8, imp);

            DECODE_OP_CODE_DIRECT(LDX, 0xA2, imm);
            DECODE_OP_CODE_DIRECT(LDX, 0xA6, zp);
            DECODE_OP_CODE_DIRECT(LDX, 0xB6, zp_ind_y);
            DECODE_OP_CODE_DIRECT(LDX, 0xAE, abs);
            DECODE_OP_CODE_DIRECT(LDX, 0xBE, abs_y);

            DECODE_OP_CODE_DIRECT(LDY, 0xA0, imm);
            DECODE_OP_CODE_DIRECT(LDY, 0xA4, zp);
            DECODE_OP_CODE_DIRECT(LDY, 0xB4, zp_ind_x);
            DECODE_OP_CODE_DIRECT(LDY, 0xAC, abs);
            DECODE_OP_CODE_DIRECT(LDY, 0xBC, abs_x);

            DECODE_OP_CODE_DIRECT(CPX, 0xE0, imm);
            DECODE_OP_CODE_DIRECT(CPX, 0xE4, zp);
            DECODE_OP_CODE_DIRECT(CPX, 0xEC, abs);

            DECODE_OP_CODE_DIRECT(CPY, 0xC0, imm);
            DECODE_OP_CODE_DIRECT(CPY, 0xC4, zp);
            DECODE_OP_CODE_DIRECT(CPY, 0xCC, abs);

            DECODE_OP_CODE_DIRECT(DEC, 0xC6, zp);
            DECODE_OP_CODE_DIRECT(DEC, 0xD6, zp_ind_x);
            DECODE_OP_CODE_DIRECT(DEC, 0xCE, abs);
            DECODE_OP_CODE_DIRECT(DEC, 0xDE, abs_x);

            DECODE_OP_CODE_DIRECT(DEX, 0xCA, imp);
            DECODE_OP_CODE_DIRECT(DEY, 0x88, imp);

            DECODE_OP_CODE_DIRECT(INC, 0xE6, zp);
            DECODE_OP_CODE_DIRECT(INC, 0xF6, zp_ind_x);
            DECODE_OP_CODE_DIRECT(INC, 0xEE, abs);
            DECODE_OP_CODE_DIRECT(INC, 0xFE, abs_x);


            DECODE_OP_CODE_DIRECT(INX, 0xE8, imp);
            DECODE_OP_CODE_DIRECT(INY, 0xC8, imp);

            //TODO Continue implementing more operations

            default:
                cout << "Invalid opcode: "<< opcode << endl;
                break;
        }

        pc += 1;

        if (pc >= getROM().size()) {
            break;
        }
    }
}

/*
int main() {

// TODO: Finish Rom.cpp by parsing PRG and CHR to class.
//spdlog::set_level(spdlog::level::debug);

Nes nes;
nes.LoadROM("roms/test/nestest.nes");
nes.loop();

}
*/