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
//#include "include/spdlog/spdlog.h"

using namespace std;

Nes::Nes() {
    //Init RNG
    unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
    randGen.seed(seed);
    randByte = uniform_int_distribution<uint8_t>(0, 255U);

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

void Nes::setOverflowFlag(uint8_t val) { setFlag(5, val); }

void Nes::setNegativeFlag(uint8_t val) { setFlag(6, val); }

int Nes::getCarryFlag() { return getFlag(0); }

int Nes::getZeroFlag() { return getFlag(1); }

int Nes::getInterruptFlag() { return getFlag(2); }

int Nes::getDecimalModeFlag() { return getFlag(3); }

int Nes::getBreakFlag() { return getFlag(4); }

int Nes::getOverflowFlag() { return getFlag(5); }

int Nes::getNegativeFlag() { return getFlag(6); }


void Nes::reset(bool test) {
    pc = 0;
    memset(stack, 0, sizeof(stack));
    memset(memory, 0, sizeof(memory));
    memset(memory, 0, sizeof(memory));
    sp = 0;
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
            DECODE_ALU_OP_CODE(ORA)// Done
            DECODE_ALU_OP_CODE(AND)// Done
            DECODE_ALU_OP_CODE(EOR)// Done
            DECODE_ALU_OP_CODE(ADC) // Done
            DECODE_ALU_OP_CODE_NO_IMM(STA)// Done
            DECODE_ALU_OP_CODE(LDA)// Done
            DECODE_ALU_OP_CODE(CMP)// Done
            DECODE_ALU_OP_CODE(SBC)// Done

            DECODE_RMW_OP_CODE(ASL);// Done
            DECODE_RMW_OP_CODE(ROL);// Done
            DECODE_RMW_OP_CODE(LSR);// Done
            DECODE_RMW_OP_CODE(ROR);// Done
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