//
// Created by miguelcoelho on 06-09-2022.
//

#include "Nes.h"
#include <functional>
#include <iostream>
#include <tuple>

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

// LDA
void Nes::set_LDA_flags() {
    if (reg_A == 0) {
        setCarryFlag(1);
    }
    if (Nes::getBit(6, reg_A)) {
        setNegativeFlag(1);
    }
}


uint8_t Nes::decode_operand(nes_addr_mode addrMode){
    uint8_t low;
    uint8_t high;
    switch (addrMode) {
        case nes_addr_mode_imp:
            break;
        case nes_addr_mode_abs:
            return memory[getNext16Code()];
        case nes_addr_mode_acc:
            break;
        case nes_addr_mode_imm:
            return getNextCode();
        case nes_addr_mode_ind_jmp:
            break;
        case nes_addr_mode_rel:
            break;
        case nes_addr_mode_abs_jmp:
            break;
        case nes_addr_mode_zp:
            return memory[getNextCode()];
        case nes_addr_mode_zp_ind_x:
            return memory[getNextCode() + reg_X];
        case nes_addr_mode_zp_ind_y:
            break;
        case nes_addr_mode_abs_x:
            return memory[getNext16Code() + reg_X];
        case nes_addr_mode_abs_y:
            return memory[getNext16Code() + reg_Y];
        case nes_addr_mode_ind_x:
            low = memory[getNextCode()];
            high = memory[getNextCode() + 0x04];
            return memory[get16bitfrom8bit(high, low) + reg_Y];
        case nes_addr_mode_ind_y:
            low = memory[getNextCode() + reg_X];
            high = memory[getNextCode() + 0x04 + reg_X];
            return memory[get16bitfrom8bit(high, low)];
        default:
            break;
    }
    return 0;
}

void Nes::setPCOffset(nes_addr_mode addrMode){
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

void Nes::LDA(nes_addr_mode addrMode){
    uint8_t operand = decode_operand(addrMode);
    reg_A = operand;
    set_LDA_flags();
}

// TODO: Do tests for each opcode