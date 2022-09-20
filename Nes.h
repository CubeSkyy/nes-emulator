//
// Created by miguelcoelho on 11-07-2022.
//

#ifndef NES_H
#define NES_H

#include <cstdint>
#include <random>
#include "Rom.h"
#include "map"
#include "functional"

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

    enum nes_op_code
    {
        ORA_base = 0x00,
        AND_base = 0x20,
        EOR_base = 0x40,
        ADC_base = 0x60,
        STA_base = 0x80,
        LDA_base = 0xA0,
        CMP_base = 0xC0,
        SBC_base = 0xE0,
    };

    enum nes_addr_mode
    {
        nes_addr_mode_imp,        // implicit
        nes_addr_mode_acc,        //          val = A
        nes_addr_mode_imm,        //          val = arg_8
        nes_addr_mode_ind_jmp,    //          val = peek16(arg_16), with JMP bug
        nes_addr_mode_rel,        //          val = arg_8, as offset
        nes_addr_mode_abs,        //          val = PEEK(arg_16), LSB then MSB
        nes_addr_mode_abs_jmp,    //          val = arg_16, LSB then MSB, direct jump address
        nes_addr_mode_zp,         //          val = PEEK(arg_8)
        nes_addr_mode_zp_ind_x,   // d, x     val = PEEK((arg_8 + X) % $FF ), 4 cycles
        nes_addr_mode_zp_ind_y,   // d, y     val = PEEK((arg_8 + Y) % $FF), 4 cycles
        nes_addr_mode_abs_x,      // a, x     val = PEEK(arg_16 + Y), 4+ cycles
        nes_addr_mode_abs_y,      // a, y     val = PEEK(arg_16 + Y), 4+ cycles
        nes_addr_mode_ind_x,      // (d, x)   val = PEEK(PEEK((arg + X) % $FF) + PEEK((arg + X + 1) % $FF) * $FF), 6 cycles
        nes_addr_mode_ind_y,      // (d), y   val = PEEK(PEEK(arg) + PEEK((arg + 1) % $FF)* $FF + Y), 5+ cycles
    };


#define DECODE_OP_CODE_(op, offset, mode) \
    case nes_op_code::op##_base + offset :    \
        op(nes_addr_mode::nes_addr_mode_##mode);  \
        setPCOffset(nes_addr_mode::nes_addr_mode_##mode);                            \
        break;

#define DECODE_OP_CODE(op) \
    DECODE_OP_CODE_(op, 0x9, imm) \
    DECODE_OP_CODE_(op, 0x5, zp) \
    DECODE_OP_CODE_(op, 0x15, zp_ind_x) \
    DECODE_OP_CODE_(op, 0xd, abs) \
    DECODE_OP_CODE_(op, 0x1d, abs_x) \
    DECODE_OP_CODE_(op, 0x19, abs_y) \
    DECODE_OP_CODE_(op, 0x1, ind_x) \
    DECODE_OP_CODE_(op, 0x11, ind_y)


    void LoadROM(char const *filename);
    vector<uint8_t> getROM();

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


    struct funcs;

    typedef void (funcs::*fun_map)(Nes*);
    typedef std::map<uint8_t , fun_map> math_func_map_t;
    math_func_map_t mapping;

    void set_LDA_flags();
    uint8_t getNextCode();
    uint16_t getNext16Code();
    uint16_t get16bitfrom8bit(uint8_t lower, uint8_t higher);



    void loop();

    void LDA(nes_addr_mode addrMode);

    uint8_t decode_operand(nes_addr_mode addrMode);

    void setPCOffset(nes_addr_mode addrMode);

    void reset(bool test);
};


#endif //NES_H
