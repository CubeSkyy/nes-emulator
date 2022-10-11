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

uint16_t Nes::getNext16Code() {
    return (getROM()[pc + 1] << 8) | (getROM()[pc + 2] & 0xff);
}
// Instructions

Nes::addr_or_value Nes::decode_operand(nes_addr_mode addrMode) {
    uint8_t low;
    uint8_t high;
    switch (addrMode) {
        case nes_addr_mode_imp:
            break;
        case nes_addr_mode_abs:
            return {getNext16Code(), op_type::op_type_addr};
        case nes_addr_mode_acc:
            return {0, op_type::op_type_acc};
        case nes_addr_mode_imm:
            return {getNextCode(), op_type::op_type_imm};
        case nes_addr_mode_ind_jmp:
            break;
        case nes_addr_mode_rel:
            break;
        case nes_addr_mode_abs_jmp:
            break;
        case nes_addr_mode_zp:
            return {getNextCode(), op_type::op_type_addr};
        case nes_addr_mode_zp_ind_x:
            return {(uint16_t) (getNextCode() + reg_X), op_type::op_type_addr};
        case nes_addr_mode_zp_ind_y:
            break;
        case nes_addr_mode_abs_x:
            return {(uint16_t) (getNext16Code() + reg_X), op_type::op_type_addr};
        case nes_addr_mode_abs_y:
            return {(uint16_t) (getNext16Code() + reg_Y), op_type::op_type_addr};
        case nes_addr_mode_ind_x:
            low = memory[getNextCode() + reg_X];
            high = memory[getNextCode() + 0x04 + reg_X];
            return {get16bitfrom8bit(high, low), op_type::op_type_addr};
        case nes_addr_mode_ind_y:
            low = memory[getNextCode()];
            high = memory[getNextCode() + 0x04];
            return {(uint16_t) (get16bitfrom8bit(high, low) + reg_Y), op_type::op_type_addr};

        default:
            return {0, op_type::op_type_addr};

    }

    return {};
}

uint8_t Nes::read_addr_or_value(Nes::addr_or_value value) {
    op_type opType = value.type;
    switch (opType) {
        case op_type_acc:
            return reg_A;
        case op_type_imm:
            return value.addr_or_value;
        case op_type_addr:
            return memory[value.addr_or_value];
    }
    return 0;
}

void Nes::write_addr_or_value(Nes::addr_or_value value, uint8_t newValue) {
    op_type opType = value.type;
    switch (opType) {
        case op_type_acc:
            reg_A = newValue;
            break;
        case op_type_addr:
            memory[value.addr_or_value] = newValue;
            break;
        default:
            assert(false);
    }
}


void Nes::setPCOffset(nes_addr_mode addrMode) {
    int offset = 0;

    switch (addrMode) {

        case nes_addr_mode_imp:
        case nes_addr_mode_acc:
            break;

        case nes_addr_mode_imm:
        case nes_addr_mode_rel: //??? Maybe
        case nes_addr_mode_zp:
        case nes_addr_mode_zp_ind_x:
        case nes_addr_mode_zp_ind_y:
        case nes_addr_mode_ind_x:
        case nes_addr_mode_ind_y:
        case nes_addr_mode_ind_jmp: //??? Maybe

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
    }
}

void Nes::setAndCheckOverflowFlag(uint8_t result, uint8_t reg) {
    if (getBit(7, result) != getBit(7, reg)) {
        setOverflowFlag(1);
    }
}

// ALU
void Nes::ORA(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    reg_A |= value;

    setAndCheckNegativeFlag(reg_A);
    setAndCheckZeroFlag(reg_A);
}


void Nes::AND(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    reg_A &= value;

    setAndCheckNegativeFlag(reg_A);
    setAndCheckZeroFlag(reg_A);
}

void Nes::EOR(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    reg_A ^= value;

    setAndCheckNegativeFlag(reg_A);
    setAndCheckZeroFlag(reg_A);
}

void Nes::ADC(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

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
    addr_or_value addrOrValue = decode_operand(addrMode);
    assert(addrOrValue.type == op_type_addr);

    memory[addrOrValue.addr_or_value] = reg_A;
}

void Nes::LDA(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    reg_A = value;

    setAndCheckZeroFlag(reg_A);
    setAndCheckNegativeFlag(reg_A);
}

void Nes::CMP(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    uint8_t temp = reg_A - value;

    setAndCheckNegativeFlag(temp);
    setCarryFlag(reg_A > temp);
    setAndCheckZeroFlag(temp);
}

void Nes::SBC(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    int16_t temp;
    if (getDecimalModeFlag()){
        temp = bcd(reg_A) - bcd(value) - (1 - getCarryFlag());
        setOverflowFlag(temp > 99 or temp < 0 );
    }else{
        temp = reg_A - value - (1 - getCarryFlag());
        setOverflowFlag(temp > 127 or temp < -128 );
    }
    setCarryFlag(temp >= 0);
    setAndCheckNegativeFlag(temp);
    setAndCheckZeroFlag(temp);

    reg_A = (uint8_t) temp;
}

// RMW
void Nes::ASL(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    setCarryFlag(getBit(7, value));
    value <<= 1;
    setAndCheckNegativeFlag(value);
    setAndCheckZeroFlag(value);

    write_addr_or_value(addrOrValue, value);

}

void Nes::ROL(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    uint8_t temp = getBit(7, value);
    value <<= 1;
    value = setBit(value, 0, getCarryFlag());
    setCarryFlag(temp);

    setAndCheckNegativeFlag(value);
    setAndCheckZeroFlag(value);

    write_addr_or_value(addrOrValue, value);
}


void Nes::LSR(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    setCarryFlag(getBit(0, value));
    value >>= 1;

    setNegativeFlag(0);
    setAndCheckZeroFlag(value);

    write_addr_or_value(addrOrValue, value);

}

void Nes::ROR(nes_addr_mode addrMode) {
    addr_or_value addrOrValue = decode_operand(addrMode);
    uint8_t value = read_addr_or_value(addrOrValue);

    uint8_t temp = getBit(0, value);
    value >>= 1;
    value = setBit(value, 7, getCarryFlag());
    setCarryFlag(temp);

    setAndCheckNegativeFlag(value);
    setAndCheckZeroFlag(value);

    write_addr_or_value(addrOrValue, value);

}