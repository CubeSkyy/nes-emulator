//
// Created by miguelcoelho on 06-09-2022.
//

#include "Nes.h"
#include <functional>
#include <iostream>
#include <tuple>
#include <cassert>

using namespace std;

// Util functions
uint8_t Nes::getNextCode() {
    return getROM()[pc + 1];
}

uint16_t Nes::get16bitfrom8bit(uint8_t higher, uint8_t lower) {
    return (higher << 8) | (lower & 0xff);
}

uint8_t Nes::getLowerBitsFrom16bit(uint16_t value) {
    return value & 0xFF;
}

uint8_t Nes::getHigherBitsFrom16bit(uint16_t value) {
    return (value >> 8) & 0xFF;
}


uint16_t Nes::getNext16Code() {
    return (getROM()[pc + 1] << 8) | (getROM()[pc + 2] & 0xff);
}
// Instructions

Nes::addr_value Nes::decode_operand(Nes::nes_addr_mode addrMode) {
    uint16_t addr;
    switch (addrMode) {
        case nes_addr_mode_imp:
            break;
        case nes_addr_mode_abs:
            addr = getNext16Code();
            return addr_value{addr, _ram->getByte(addr)};
        case nes_addr_mode_ind_jmp:
            addr = _ram->getWord(getNextCode());
            return addr_value{addr, _ram->getByte(addr)};
        case nes_addr_mode_abs_jmp:
            return addr_value{addr, addr};
        case nes_addr_mode_acc:
            return addr_value{addr, reg_A};
        case nes_addr_mode_imm:
        case nes_addr_mode_rel:
            return addr_value{addr, getNextCode()};
        case nes_addr_mode_zp:
            addr = getNextCode();
            return addr_value{addr, _ram->getByte(addr)};
        case nes_addr_mode_zp_ind_x:
            addr = getNextCode() + reg_X;
            return addr_value{addr, _ram->getByte(addr)};
        case nes_addr_mode_zp_ind_y:
            addr = getNextCode() + reg_Y;
            return addr_value{addr, _ram->getByte(addr)};
        case nes_addr_mode_abs_x:
            addr = getNext16Code() + reg_X;
            return addr_value{addr, _ram->getByte(addr)};
        case nes_addr_mode_abs_y:
            addr = getNext16Code() + reg_Y;
            return addr_value{addr, _ram->getByte(addr)};
        case nes_addr_mode_ind_x:
            addr = _ram->getWord(getNextCode() + reg_X);
            return addr_value{addr, _ram->getByte(addr)};
        case nes_addr_mode_ind_y:
            addr = _ram->getWord(getNextCode()) + reg_Y;
            return addr_value{addr, _ram->getByte(addr)};
    }
}



void Nes::setPCOffset(nes_addr_mode addrMode) {
    int offset = 0;

    switch (addrMode) {

        case nes_addr_mode_acc:
        case nes_addr_mode_imp:
            break;

        case nes_addr_mode_imm:
        case nes_addr_mode_zp:
        case nes_addr_mode_zp_ind_x:
        case nes_addr_mode_zp_ind_y:
        case nes_addr_mode_ind_x:
        case nes_addr_mode_ind_y:
        case nes_addr_mode_ind_jmp: //??? Maybe
        case nes_addr_mode_rel:
            offset = 1;
            break;

        case nes_addr_mode_abs:
        case nes_addr_mode_abs_x:
        case nes_addr_mode_abs_y:
        case nes_addr_mode_abs_jmp:
            offset = 2;
            break;
    }
    pc += offset;
}

uint16_t bcd(uint16_t h) {
    uint16_t d = 0;
    int power = 1;
    while (h) {
        d += (h & 15) * power;
        h >>= 4;
        power *= 10;
    }
    return d;
}

void Nes::setAndCheckNegativeFlag(uint8_t reg) {
    setNegativeFlag(getBit(7, reg));
}

void Nes::setAndCheckZeroFlag(uint8_t reg) {
    if (reg == 0) {
        setZeroFlag(1);
    } else {
        setZeroFlag(0);
    }
}

void Nes::setAndCheckOverflowFlag(uint8_t result, uint8_t reg) {
    if (getBit(7, result) != getBit(7, reg)) {
        setOverflowFlag(1);
    }
}

// ALU
void Nes::ORA(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_A |= value;

    setAndCheckNegativeFlag(reg_A);
    setAndCheckZeroFlag(reg_A);
}


void Nes::AND(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_A &= value;

    setAndCheckNegativeFlag(reg_A);
    setAndCheckZeroFlag(reg_A);
}

void Nes::EOR(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_A ^= value;

    setAndCheckNegativeFlag(reg_A);
    setAndCheckZeroFlag(reg_A);
}

void Nes::ADC(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    uint16_t temp = reg_A + value + getCarryFlag();

    setAndCheckOverflowFlag(temp, reg_A);
    setAndCheckNegativeFlag(reg_A);
    setAndCheckZeroFlag(temp);

    if (getDecimalModeFlag()) {
        temp = bcd(reg_A) + bcd(value) + getCarryFlag();
        setCarryFlag(temp > 99);
    } else {
        setCarryFlag(temp > 255);
    }
    reg_A = temp;
}

void Nes::STA(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);
    _ram->setByte(addr, reg_A);
}

void Nes::LDA(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_A = value;

    setAndCheckZeroFlag(reg_A);
    setAndCheckNegativeFlag(reg_A);
}

void Nes::LDX(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_X = value;

    setAndCheckZeroFlag(reg_X);
    setAndCheckNegativeFlag(reg_X);
}

void Nes::LDY(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_Y = value;

    setAndCheckZeroFlag(reg_Y);
    setAndCheckNegativeFlag(reg_Y);
}

void Nes::CMP(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    uint8_t temp = reg_A - value;

    setAndCheckNegativeFlag(temp);
    setCarryFlag(reg_A > temp);
    setAndCheckZeroFlag(temp);
}

void Nes::SBC(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    int16_t temp;
    if (getDecimalModeFlag()) {
        temp = bcd(reg_A) - bcd(value) - (1 - getCarryFlag());
        setOverflowFlag(temp > 99 or temp < 0);
    } else {
        temp = reg_A - value - (1 - getCarryFlag());
        setOverflowFlag(temp > 127 or temp < -128);
    }
    setCarryFlag(temp >= 0);
    setAndCheckNegativeFlag(temp);
    setAndCheckZeroFlag(temp);

    reg_A = (uint8_t) temp;
}

// RMW
void Nes::ASL(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    setCarryFlag(getBit(7, value));
    value <<= 1;
    setAndCheckNegativeFlag(value);
    setAndCheckZeroFlag(value);

    write_uint8_t(value, value);

}

void Nes::ROL(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    uint8_t temp = getBit(7, value);
    value <<= 1;
    value = setBit(value, 0, getCarryFlag());
    setCarryFlag(temp);

    setAndCheckNegativeFlag(value);
    setAndCheckZeroFlag(value);

    write_uint8_t(value, value);
}


void Nes::LSR(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    setCarryFlag(getBit(0, value));
    value >>= 1;

    setNegativeFlag(0);
    setAndCheckZeroFlag(value);

    write_uint8_t(value, value);

}

void Nes::ROR(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    uint8_t temp = getBit(0, value);
    value >>= 1;
    value = setBit(value, 7, getCarryFlag());
    setCarryFlag(temp);

    setAndCheckNegativeFlag(value);
    setAndCheckZeroFlag(value);

    write_uint8_t(value, value);
}

void Nes::BCC(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    if (getCarryFlag() == 0) {
        pc += value - 2;
    }
}

void Nes::BCS(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    if (getCarryFlag() == 1) {
        pc += value - 2;
    }
}

void Nes::BEQ(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    if (getZeroFlag() == 1) {
        pc += value - 2;
    }
}

void Nes::BIT(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    uint8_t temp = value & reg_A;

    setAndCheckZeroFlag(temp);
    setAndCheckNegativeFlag(temp);
    setOverflowFlag(getBit(6, temp));
}

void Nes::BMI(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    if (getNegativeFlag() == 1) {
        pc += value - 2;
    }
}

void Nes::BNE(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    if (getZeroFlag() == 0) {
        pc += value - 2;
    }
}

void Nes::BPL(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    if (getNegativeFlag() == 0) {
        pc += value - 2;
    }
}


void Nes::BRK(nes_addr_mode addrMode) {
    pc++;

    // Push PC.h to the stack
    pushToStack(getHigherBitsFrom16bit(pc));

    // Push PC.l to the stack
    pushToStack(getLowerBitsFrom16bit(pc));

    // Push status register (P) with bit 4 (Break flag) set
    pushToStack(reg_stat | (1 << 4));

    // Load the interrupt vector to PC
    pc = (memory[0xFFFF] << 8) | memory[0xFFFE];
}

void Nes::BVC(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    if (getOverflowFlag() == 0) {
        pc += value - 2;
    }
}

void Nes::BVS(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    if (getOverflowFlag() == 1) {
        pc += value - 2;
    }
}

void Nes::CLC(nes_addr_mode addrMode) {
    setCarryFlag(0);
}

void Nes::CLD(nes_addr_mode addrMode) {
    setDecimalModeFlag(0);
}

void Nes::CLI(nes_addr_mode addrMode) {
    setInterruptFlag(0);
}

void Nes::CLV(nes_addr_mode addrMode) {
    setOverflowFlag(0);
}

void Nes::CPX(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    uint8_t temp = reg_X - value;

    setAndCheckNegativeFlag(temp);
    setCarryFlag(reg_X > temp);
    setAndCheckZeroFlag(temp);
}

void Nes::CPY(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    uint8_t temp = reg_Y - value;

    setAndCheckNegativeFlag(temp);
    setCarryFlag(reg_Y > temp);
    setAndCheckZeroFlag(temp);
}

void Nes::DEC(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    memory[value.uint8_t] = --value;

    setAndCheckNegativeFlag(value);
    setAndCheckZeroFlag(value);
}

void Nes::DEX(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_X -= 1;

    setAndCheckNegativeFlag(reg_X);
    setAndCheckZeroFlag(reg_X);
}

void Nes::DEY(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_Y -= 1;

    setAndCheckNegativeFlag(reg_Y);
    setAndCheckZeroFlag(reg_Y);
}

void Nes::INC(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    memory[value.uint8_t] = ++value;

    setAndCheckNegativeFlag(value);
    setAndCheckZeroFlag(value);
}


void Nes::INX(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_X += 1;

    setAndCheckNegativeFlag(reg_X);
    setAndCheckZeroFlag(reg_X);
}

void Nes::INY(nes_addr_mode addrMode) {
    auto [addr, value] = decode_operand(addrMode);

    reg_Y += 1;

    setAndCheckNegativeFlag(reg_Y);
    setAndCheckZeroFlag(reg_Y);
}
