#pragma once

#include <cstdint>
#include <array>
#include <set>
#include <string>

#include "cartridge/cartridge.h"

class Bus; // Forward declaration

class CPU {
public:
    // =====================
    // === Types & Consts ===
    // =====================
    struct Operation {
        const char* name_;
        void (CPU::*op_function_)();
        void (CPU::*addr_mode_)();
        uint8_t cycles_;
    };

    enum Flags {
        C = 0, // Carry
        Z = 1, // Zero
        I = 2, // Interrupt Disable
        D = 3, // Decimal Mode (not used in most emulators)
        B = 4, // Break Command
        R = 5, // Reserved
        V = 6, // Overflow
        N = 7 // Negative
    };

    // =====================
    // === Public API ======
    // =====================

    CPU();
    ~CPU() = default;

    // CPU memory access
    [[nodiscard]] uint8_t Read(uint16_t addr) const;
    void Write(uint16_t addr, uint8_t data) const;

    // Emulation step
    void Step();
    void StepInstruction();
    void Reset();
    void IRQ();
    void NMI();
    void RTI();

    // Addressing modes
    void ADR_IMP(), ADR_IMM(), ADR_REL(), ADR_ZP0(),
         ADR_ZPX(), ADR_ZPY(), ADR_ABS(), ADR_ABX(),
         ADR_ABY(), ADR_IND(), ADR_IZX(), ADR_IZY();

private:
    // =====================
    // === NES Registers ===
    // =====================
    uint8_t a_, x_, y_, sp_;
    uint16_t pc_;
    uint8_t p_; // Flags register

    // Cycles
    uint8_t current_cycle_;
    uint32_t total_cycles_;

    // Addressing fetch variables
    uint16_t fetched_address_;
    void (CPU::*current_addr_mode_)();
    Operation current_operation_;

    // Linkto  bus
    Bus* bus_ = nullptr;

    // Opcodes
    void OP_ADC(), OP_AND(), OP_ASL(), OP_BCC(), OP_BCS(), OP_BEQ(), OP_BIT(), OP_BMI(), OP_BNE(), OP_BPL();
    void OP_BRK(), OP_BVC(), OP_BVS(), OP_CLC(), OP_CLD(), OP_CLI(), OP_CLV(), OP_CMP(), OP_CPX(), OP_CPY();
    void OP_DEC(), OP_DEX(), OP_DEY(), OP_EOR(), OP_INC(), OP_INX(), OP_INY(), OP_JMP(), OP_JSR(), OP_LDA();
    void OP_LDX(), OP_LDY(), OP_LSR(), OP_NOP(), OP_ORA(), OP_PHA(), OP_PHP(), OP_PLA(), OP_PLP(), OP_ROL();
    void OP_ROR(), OP_RTS(), OP_SBC(), OP_SEC(), OP_SED(), OP_SEI(), OP_STA(), OP_STX(), OP_STY(), OP_TAX();
    void OP_TAY(), OP_TSX(), OP_TXA(), OP_TXS(), OP_TYA(), OP_LAX(), OP_SAX(), OP_DCP(), OP_SLO(), OP_ANC();
    void OP_RLA(), OP_SRE(), OP_RRA(), OP_XAA(), OP_TAS(), OP_SHY(), OP_SHX(), OP_LXA(), OP_LAS(), OP_AXS();
    void OP_ISC(), OP_ARR(), OP_ASR();
    void OP_UNF();

    // Store instructions list, used in page crossing checks
    const std::set<std::string> kStoreOps = {
        "STA", "STX", "STY", "SAX", "SHA", "SHX", "SHY", "TAS", "ISC",
        "SLO", "RLA", "SRE", "RRA", "DCP", "ISB"
    };

    static constexpr std::array<Operation, 256> kOpcodeTable = {
        {
            /* 0x0X */
            /* 0x00 */ {"BRK", &CPU::OP_BRK, &CPU::ADR_IMP, 7},
            /* 0x01 */ {"ORA", &CPU::OP_ORA, &CPU::ADR_IZX, 6},
            /* 0x02 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x03 */ {"SLO", &CPU::OP_SLO, &CPU::ADR_IZX, 8}, // unofficial
            /* 0x04 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZP0, 3},
            /* 0x05 */ {"ORA", &CPU::OP_ORA, &CPU::ADR_ZP0, 3},
            /* 0x06 */ {"ASL", &CPU::OP_ASL, &CPU::ADR_ZP0, 5},
            /* 0x07 */ {"SLO", &CPU::OP_SLO, &CPU::ADR_ZP0, 5}, // unofficial
            /* 0x08 */ {"PHP", &CPU::OP_PHP, &CPU::ADR_IMP, 3},
            /* 0x09 */ {"ORA", &CPU::OP_ORA, &CPU::ADR_IMM, 2},
            /* 0x0A */ {"ASL", &CPU::OP_ASL, &CPU::ADR_IMP, 2},
            /* 0x0B */ {"ANC", &CPU::OP_ANC, &CPU::ADR_IMM, 2}, // unofficial
            /* 0x0C */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ABS, 4},
            /* 0x0D */ {"ORA", &CPU::OP_ORA, &CPU::ADR_ABS, 4},
            /* 0x0E */ {"ASL", &CPU::OP_ASL, &CPU::ADR_ABS, 6},
            /* 0x0F */ {"SLO", &CPU::OP_SLO, &CPU::ADR_ABS, 6}, // unofficial

            /* 0x1X */
            /* 0x10 */ {"BPL", &CPU::OP_BPL, &CPU::ADR_REL, 2},
            /* 0x11 */ {"ORA", &CPU::OP_ORA, &CPU::ADR_IZY, 5},
            /* 0x12 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x13 */ {"SLO", &CPU::OP_SLO, &CPU::ADR_IZY, 8}, // unofficial
            /* 0x14 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZPX, 4},
            /* 0x15 */ {"ORA", &CPU::OP_ORA, &CPU::ADR_ZPX, 4},
            /* 0x16 */ {"ASL", &CPU::OP_ASL, &CPU::ADR_ZPX, 6},
            /* 0x17 */ {"SLO", &CPU::OP_SLO, &CPU::ADR_ZPX, 6}, // unofficial
            /* 0x18 */ {"CLC", &CPU::OP_CLC, &CPU::ADR_IMP, 2},
            /* 0x19 */ {"ORA", &CPU::OP_ORA, &CPU::ADR_ABY, 4},
            /* 0x1A */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMP, 2},
            /* 0x1B */ {"SLO", &CPU::OP_SLO, &CPU::ADR_ABY, 7}, // unofficial
            /* 0x1C */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ABX, 4},
            /* 0x1D */ {"ORA", &CPU::OP_ORA, &CPU::ADR_ABX, 4},
            /* 0x1E */ {"ASL", &CPU::OP_ASL, &CPU::ADR_ABX, 7},
            /* 0x1F */ {"SLO", &CPU::OP_SLO, &CPU::ADR_ABX, 7}, // unofficial

            /* 0x2X */
            /* 0x20 */ {"JSR", &CPU::OP_JSR, &CPU::ADR_ABS, 6},
            /* 0x21 */ {"AND", &CPU::OP_AND, &CPU::ADR_IZX, 6},
            /* 0x22 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x23 */ {"RLA", &CPU::OP_RLA, &CPU::ADR_IZX, 8}, // unofficial
            /* 0x24 */ {"BIT", &CPU::OP_BIT, &CPU::ADR_ZP0, 3},
            /* 0x25 */ {"AND", &CPU::OP_AND, &CPU::ADR_ZP0, 3},
            /* 0x26 */ {"ROL", &CPU::OP_ROL, &CPU::ADR_ZP0, 5},
            /* 0x27 */ {"RLA", &CPU::OP_RLA, &CPU::ADR_ZP0, 5}, // unofficial
            /* 0x28 */ {"PLP", &CPU::OP_PLP, &CPU::ADR_IMP, 4},
            /* 0x29 */ {"AND", &CPU::OP_AND, &CPU::ADR_IMM, 2},
            /* 0x2A */ {"ROL", &CPU::OP_ROL, &CPU::ADR_IMP, 2},
            /* 0x2B */ {"ANC", &CPU::OP_ANC, &CPU::ADR_IMP, 2}, // unofficial
            /* 0x2C */ {"BIT", &CPU::OP_BIT, &CPU::ADR_ABS, 4},
            /* 0x2D */ {"AND", &CPU::OP_AND, &CPU::ADR_ABS, 4},
            /* 0x2E */ {"ROL", &CPU::OP_ROL, &CPU::ADR_ABS, 6},
            /* 0x2F */ {"RLA", &CPU::OP_RLA, &CPU::ADR_ABS, 6}, // unofficial

            /* 0x3X */
            /* 0x30 */ {"BMI", &CPU::OP_BMI, &CPU::ADR_REL, 2},
            /* 0x31 */ {"AND", &CPU::OP_AND, &CPU::ADR_IZY, 5},
            /* 0x32 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x33 */ {"RLA", &CPU::OP_RLA, &CPU::ADR_IZY, 8}, // unofficial
            /* 0x34 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZPX, 4},
            /* 0x35 */ {"AND", &CPU::OP_AND, &CPU::ADR_ZPX, 4},
            /* 0x36 */ {"ROL", &CPU::OP_ROL, &CPU::ADR_ZPX, 6},
            /* 0x37 */ {"RLA", &CPU::OP_RLA, &CPU::ADR_ZPX, 6}, // unofficial
            /* 0x38 */ {"SEC", &CPU::OP_SEC, &CPU::ADR_IMP, 2},
            /* 0x39 */ {"AND", &CPU::OP_AND, &CPU::ADR_ABY, 4},
            /* 0x3A */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMP, 2},
            /* 0x3B */ {"RLA", &CPU::OP_RLA, &CPU::ADR_ABY, 7}, // unofficial
            /* 0x3C */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ABX, 4},
            /* 0x3D */ {"AND", &CPU::OP_AND, &CPU::ADR_ABX, 4},
            /* 0x3E */ {"ROL", &CPU::OP_ROL, &CPU::ADR_ABX, 7},
            /* 0x3F */ {"RLA", &CPU::OP_RLA, &CPU::ADR_ABX, 7}, // unofficial

            /* 0x4X */
            /* 0x40 */ {"RTI", &CPU::RTI, &CPU::ADR_IMP, 6},
            /* 0x41 */ {"EOR", &CPU::OP_EOR, &CPU::ADR_IZX, 6},
            /* 0x42 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x43 */ {"SRE", &CPU::OP_SRE, &CPU::ADR_IZX, 8}, // unofficial
            /* 0x44 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZP0, 3},
            /* 0x45 */ {"EOR", &CPU::OP_EOR, &CPU::ADR_ZP0, 3},
            /* 0x46 */ {"LSR", &CPU::OP_LSR, &CPU::ADR_ZP0, 5},
            /* 0x47 */ {"SRE", &CPU::OP_SRE, &CPU::ADR_ZP0, 5}, // unofficial
            /* 0x48 */ {"PHA", &CPU::OP_PHA, &CPU::ADR_IMP, 3},
            /* 0x49 */ {"EOR", &CPU::OP_EOR, &CPU::ADR_IMM, 2},
            /* 0x4A */ {"LSR", &CPU::OP_LSR, &CPU::ADR_IMP, 2},
            /* 0x4B */ {"ASR", &CPU::OP_ASR, &CPU::ADR_IMP, 2}, // unofficial
            /* 0x4C */ {"JMP", &CPU::OP_JMP, &CPU::ADR_ABS, 3},
            /* 0x4D */ {"EOR", &CPU::OP_EOR, &CPU::ADR_ABS, 4},
            /* 0x4E */ {"LSR", &CPU::OP_LSR, &CPU::ADR_ABS, 6},
            /* 0x4F */ {"SRE", &CPU::OP_SRE, &CPU::ADR_ABS, 6}, // unofficial

            /* 0x5X */
            /* 0x50 */ {"BVC", &CPU::OP_BVC, &CPU::ADR_REL, 2},
            /* 0x51 */ {"EOR", &CPU::OP_EOR, &CPU::ADR_IZY, 5},
            /* 0x52 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x53 */ {"SRE", &CPU::OP_SRE, &CPU::ADR_IZY, 8}, // unofficial
            /* 0x54 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZPX, 4},
            /* 0x55 */ {"EOR", &CPU::OP_EOR, &CPU::ADR_ZPX, 4},
            /* 0x56 */ {"LSR", &CPU::OP_LSR, &CPU::ADR_ZPX, 6},
            /* 0x57 */ {"SRE", &CPU::OP_SRE, &CPU::ADR_ZPX, 6}, // unofficial
            /* 0x58 */ {"CLI", &CPU::OP_CLI, &CPU::ADR_IMP, 2},
            /* 0x59 */ {"EOR", &CPU::OP_EOR, &CPU::ADR_ABY, 4},
            /* 0x5A */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMP, 2},
            /* 0x5B */ {"SRE", &CPU::OP_SRE, &CPU::ADR_ABY, 7}, // unofficial
            /* 0x5C */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ABX, 4},
            /* 0x5D */ {"EOR", &CPU::OP_EOR, &CPU::ADR_ABX, 4},
            /* 0x5E */ {"LSR", &CPU::OP_LSR, &CPU::ADR_ABX, 7},
            /* 0x5F */ {"SRE", &CPU::OP_SRE, &CPU::ADR_ABX, 7}, // unofficial

            /* 0x6X */
            /* 0x60 */ {"RTS", &CPU::OP_RTS, &CPU::ADR_IMP, 6},
            /* 0x61 */ {"ADC", &CPU::OP_ADC, &CPU::ADR_IZX, 6},
            /* 0x62 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x63 */ {"RRA", &CPU::OP_RRA, &CPU::ADR_IZX, 8}, // unofficial
            /* 0x64 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZP0, 3},
            /* 0x65 */ {"ADC", &CPU::OP_ADC, &CPU::ADR_ZP0, 3},
            /* 0x66 */ {"ROR", &CPU::OP_ROR, &CPU::ADR_ZP0, 5},
            /* 0x67 */ {"RRA", &CPU::OP_RRA, &CPU::ADR_ZP0, 5}, // unofficial
            /* 0x68 */ {"PLA", &CPU::OP_PLA, &CPU::ADR_IMP, 4},
            /* 0x69 */ {"ADC", &CPU::OP_ADC, &CPU::ADR_IMM, 2},
            /* 0x6A */ {"ROR", &CPU::OP_ROR, &CPU::ADR_IMP, 2},
            /* 0x6B */ {"ARR", &CPU::OP_ARR, &CPU::ADR_IMP, 2}, // unofficial
            /* 0x6C */ {"JMP", &CPU::OP_JMP, &CPU::ADR_IND, 5},
            /* 0x6D */ {"ADC", &CPU::OP_ADC, &CPU::ADR_ABS, 4},
            /* 0x6E */ {"ROR", &CPU::OP_ROR, &CPU::ADR_ABS, 6},
            /* 0x6F */ {"RRA", &CPU::OP_RRA, &CPU::ADR_ABS, 6}, // unofficial

            /* 0x7X */
            /* 0x70 */ {"BVS", &CPU::OP_BVS, &CPU::ADR_REL, 2},
            /* 0x71 */ {"ADC", &CPU::OP_ADC, &CPU::ADR_IZY, 5},
            /* 0x72 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x73 */ {"RRA", &CPU::OP_RRA, &CPU::ADR_IZY, 8}, // unofficial
            /* 0x74 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZPX, 4},
            /* 0x75 */ {"ADC", &CPU::OP_ADC, &CPU::ADR_ZPX, 4},
            /* 0x76 */ {"ROR", &CPU::OP_ROR, &CPU::ADR_ZPX, 6},
            /* 0x77 */ {"RRA", &CPU::OP_RRA, &CPU::ADR_ZPX, 6}, // unofficial
            /* 0x78 */ {"SEI", &CPU::OP_SEI, &CPU::ADR_IMP, 2},
            /* 0x79 */ {"ADC", &CPU::OP_ADC, &CPU::ADR_ABY, 4},
            /* 0x7A */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMP, 2},
            /* 0x7B */ {"RRA", &CPU::OP_RRA, &CPU::ADR_ABY, 7}, // unofficial
            /* 0x7C */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ABX, 4},
            /* 0x7D */ {"ADC", &CPU::OP_ADC, &CPU::ADR_ABX, 4},
            /* 0x7E */ {"ROR", &CPU::OP_ROR, &CPU::ADR_ABX, 7},
            /* 0x7F */ {"RRA", &CPU::OP_RRA, &CPU::ADR_ABX, 7}, // unofficial

            /* 0x8X */
            /* 0x80 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMM, 2},
            /* 0x81 */ {"STA", &CPU::OP_STA, &CPU::ADR_IZX, 6},
            /* 0x82 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMM, 2},
            /* 0x83 */ {"SAX", &CPU::OP_SAX, &CPU::ADR_IZX, 6}, // unofficial
            /* 0x84 */ {"STY", &CPU::OP_STY, &CPU::ADR_ZP0, 3},
            /* 0x85 */ {"STA", &CPU::OP_STA, &CPU::ADR_ZP0, 3},
            /* 0x86 */ {"STX", &CPU::OP_STX, &CPU::ADR_ZP0, 3},
            /* 0x87 */ {"SAX", &CPU::OP_SAX, &CPU::ADR_ZP0, 3}, // unofficial
            /* 0x88 */ {"DEY", &CPU::OP_DEY, &CPU::ADR_IMP, 2},
            /* 0x89 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMM, 2},
            /* 0x8A */ {"TXA", &CPU::OP_TXA, &CPU::ADR_IMP, 2},
            /* 0x8B */ {"XAA", &CPU::OP_XAA, &CPU::ADR_IMM, 2}, // unofficial
            /* 0x8C */ {"STY", &CPU::OP_STY, &CPU::ADR_ABS, 4},
            /* 0x8D */ {"STA", &CPU::OP_STA, &CPU::ADR_ABS, 4},
            /* 0x8E */ {"STX", &CPU::OP_STX, &CPU::ADR_ABS, 4},
            /* 0x8F */ {"SAX", &CPU::OP_SAX, &CPU::ADR_ABS, 4}, // unofficial

            /* 0x9X */
            /* 0x90 */ {"BCC", &CPU::OP_BCC, &CPU::ADR_REL, 2},
            /* 0x91 */ {"STA", &CPU::OP_STA, &CPU::ADR_IZY, 6},
            /* 0x92 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0x93 */ {"SAX", &CPU::OP_SAX, &CPU::ADR_IZY, 6}, // unofficial
            /* 0x94 */ {"STY", &CPU::OP_STY, &CPU::ADR_ZPX, 4},
            /* 0x95 */ {"STA", &CPU::OP_STA, &CPU::ADR_ZPX, 4},
            /* 0x96 */ {"STX", &CPU::OP_STX, &CPU::ADR_ZPY, 4},
            /* 0x97 */ {"SAX", &CPU::OP_SAX, &CPU::ADR_ZPY, 4}, // unofficial
            /* 0x98 */ {"TYA", &CPU::OP_TYA, &CPU::ADR_IMP, 2},
            /* 0x99 */ {"STA", &CPU::OP_STA, &CPU::ADR_ABY, 5},
            /* 0x9A */ {"TXS", &CPU::OP_TXS, &CPU::ADR_IMP, 2},
            /* 0x9B */ {"TAS", &CPU::OP_TAS, &CPU::ADR_ABY, 5}, // unofficial
            /* 0x9C */ {"SHY", &CPU::OP_SHY, &CPU::ADR_ABX, 5}, // unofficial
            /* 0x9D */ {"STA", &CPU::OP_STA, &CPU::ADR_ABX, 5},
            /* 0x9E */ {"SHX", &CPU::OP_SHX, &CPU::ADR_ABY, 5}, // unofficial
            /* 0x9F */ {"SAX", &CPU::OP_SAX, &CPU::ADR_ABY, 5}, // unofficial

            /* 0xAX */
            /* 0xA0 */ {"LDY", &CPU::OP_LDY, &CPU::ADR_IMM, 2},
            /* 0xA1 */ {"LDA", &CPU::OP_LDA, &CPU::ADR_IZX, 6},
            /* 0xA2 */ {"LDX", &CPU::OP_LDX, &CPU::ADR_IMM, 2},
            /* 0xA3 */ {"LAX", &CPU::OP_LAX, &CPU::ADR_IZX, 6}, // unofficial
            /* 0xA4 */ {"LDY", &CPU::OP_LDY, &CPU::ADR_ZP0, 3},
            /* 0xA5 */ {"LDA", &CPU::OP_LDA, &CPU::ADR_ZP0, 3},
            /* 0xA6 */ {"LDX", &CPU::OP_LDX, &CPU::ADR_ZP0, 3},
            /* 0xA7 */ {"LAX", &CPU::OP_LAX, &CPU::ADR_ZP0, 3}, // unofficial
            /* 0xA8 */ {"TAY", &CPU::OP_TAY, &CPU::ADR_IMP, 2},
            /* 0xA9 */ {"LDA", &CPU::OP_LDA, &CPU::ADR_IMM, 2},
            /* 0xAA */ {"TAX", &CPU::OP_TAX, &CPU::ADR_IMP, 2},
            /* 0xAB */ {"LXA", &CPU::OP_LXA, &CPU::ADR_IMM, 2}, // unofficial
            /* 0xAC */ {"LDY", &CPU::OP_LDY, &CPU::ADR_ABS, 4},
            /* 0xAD */ {"LDA", &CPU::OP_LDA, &CPU::ADR_ABS, 4},
            /* 0xAE */ {"LDX", &CPU::OP_LDX, &CPU::ADR_ABS, 4},
            /* 0xAF */ {"LAX", &CPU::OP_LAX, &CPU::ADR_ABS, 4}, // unofficial

            /* 0xBX */
            /* 0xB0 */ {"BCS", &CPU::OP_BCS, &CPU::ADR_REL, 2},
            /* 0xB1 */ {"LDA", &CPU::OP_LDA, &CPU::ADR_IZY, 5},
            /* 0xB2 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0xB3 */ {"LAX", &CPU::OP_LAX, &CPU::ADR_IZY, 5}, // unofficial
            /* 0xB4 */ {"LDY", &CPU::OP_LDY, &CPU::ADR_ZPX, 4},
            /* 0xB5 */ {"LDA", &CPU::OP_LDA, &CPU::ADR_ZPX, 4},
            /* 0xB6 */ {"LDX", &CPU::OP_LDX, &CPU::ADR_ZPY, 4},
            /* 0xB7 */ {"LAX", &CPU::OP_LAX, &CPU::ADR_ZPY, 4}, // unofficial
            /* 0xB8 */ {"CLV", &CPU::OP_CLV, &CPU::ADR_IMP, 2},
            /* 0xB9 */ {"LDA", &CPU::OP_LDA, &CPU::ADR_ABY, 4},
            /* 0xBA */ {"TSX", &CPU::OP_TSX, &CPU::ADR_IMP, 2},
            /* 0xBB */ {"LAS", &CPU::OP_LAS, &CPU::ADR_ABY, 4}, // unofficial
            /* 0xBC */ {"LDY", &CPU::OP_LDY, &CPU::ADR_ABX, 4},
            /* 0xBD */ {"LDA", &CPU::OP_LDA, &CPU::ADR_ABX, 4},
            /* 0xBE */ {"LDX", &CPU::OP_LDX, &CPU::ADR_ABY, 4},
            /* 0xBF */ {"LAX", &CPU::OP_LAX, &CPU::ADR_ABY, 4}, // unofficial

            /* 0xCX */
            /* 0xC0 */ {"CPY", &CPU::OP_CPY, &CPU::ADR_IMM, 2},
            /* 0xC1 */ {"CMP", &CPU::OP_CMP, &CPU::ADR_IZX, 6},
            /* 0xC2 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMM, 2},
            /* 0xC3 */ {"DCP", &CPU::OP_DCP, &CPU::ADR_IZX, 8}, // unofficial
            /* 0xC4 */ {"CPY", &CPU::OP_CPY, &CPU::ADR_ZP0, 3},
            /* 0xC5 */ {"CMP", &CPU::OP_CMP, &CPU::ADR_ZP0, 3},
            /* 0xC6 */ {"DEC", &CPU::OP_DEC, &CPU::ADR_ZP0, 5},
            /* 0xC7 */ {"DCP", &CPU::OP_DCP, &CPU::ADR_ZP0, 5}, // unofficial
            /* 0xC8 */ {"INY", &CPU::OP_INY, &CPU::ADR_IMP, 2},
            /* 0xC9 */ {"CMP", &CPU::OP_CMP, &CPU::ADR_IMM, 2},
            /* 0xCA */ {"DEX", &CPU::OP_DEX, &CPU::ADR_IMP, 2},
            /* 0xCB */ {"AXS", &CPU::OP_AXS, &CPU::ADR_IMM, 2}, // unofficial
            /* 0xCC */ {"CPY", &CPU::OP_CPY, &CPU::ADR_ABS, 4},
            /* 0xCD */ {"CMP", &CPU::OP_CMP, &CPU::ADR_ABS, 4},
            /* 0xCE */ {"DEC", &CPU::OP_DEC, &CPU::ADR_ABS, 6},
            /* 0xCF */ {"DCP", &CPU::OP_DCP, &CPU::ADR_ABS, 6}, // unofficial

            /* 0xDX */
            /* 0xD0 */ {"BNE", &CPU::OP_BNE, &CPU::ADR_REL, 2},
            /* 0xD1 */ {"CMP", &CPU::OP_CMP, &CPU::ADR_IZY, 5},
            /* 0xD2 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0xD3 */ {"DCP", &CPU::OP_DCP, &CPU::ADR_IZY, 8}, // unofficial
            /* 0xD4 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZPX, 4},
            /* 0xD5 */ {"CMP", &CPU::OP_CMP, &CPU::ADR_ZPX, 4},
            /* 0xD6 */ {"DEC", &CPU::OP_DEC, &CPU::ADR_ZPX, 6},
            /* 0xD7 */ {"DCP", &CPU::OP_DCP, &CPU::ADR_ZPX, 6}, // unofficial
            /* 0xD8 */ {"CLD", &CPU::OP_CLD, &CPU::ADR_IMP, 2},
            /* 0xD9 */ {"CMP", &CPU::OP_CMP, &CPU::ADR_ABY, 4},
            /* 0xDA */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMP, 2},
            /* 0xDB */ {"DCP", &CPU::OP_DCP, &CPU::ADR_ABY, 7}, // unofficial
            /* 0xDC */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ABX, 4},
            /* 0xDD */ {"CMP", &CPU::OP_CMP, &CPU::ADR_ABX, 4},
            /* 0xDE */ {"DEC", &CPU::OP_DEC, &CPU::ADR_ABX, 7},
            /* 0xDF */ {"DCP", &CPU::OP_DCP, &CPU::ADR_ABX, 7}, // unofficial

            /* 0xEX */
            /* 0xE0 */ {"CPX", &CPU::OP_CPX, &CPU::ADR_IMM, 2},
            /* 0xE1 */ {"SBC", &CPU::OP_SBC, &CPU::ADR_IZX, 6},
            /* 0xE2 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMM, 2},
            /* 0xE3 */ {"ISC", &CPU::OP_ISC, &CPU::ADR_IZX, 8}, // unofficial
            /* 0xE4 */ {"CPX", &CPU::OP_CPX, &CPU::ADR_ZP0, 3},
            /* 0xE5 */ {"SBC", &CPU::OP_SBC, &CPU::ADR_ZP0, 3},
            /* 0xE6 */ {"INC", &CPU::OP_INC, &CPU::ADR_ZP0, 5},
            /* 0xE7 */ {"ISC", &CPU::OP_ISC, &CPU::ADR_ZP0, 5}, // unofficial
            /* 0xE8 */ {"INX", &CPU::OP_INX, &CPU::ADR_IMP, 2},
            /* 0xE9 */ {"SBC", &CPU::OP_SBC, &CPU::ADR_IMM, 2},
            /* 0xEA */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMP, 2},
            /* 0xEB */ {"SBC", &CPU::OP_SBC, &CPU::ADR_IMM, 2},
            /* 0xEC */ {"CPX", &CPU::OP_CPX, &CPU::ADR_ABS, 4},
            /* 0xED */ {"SBC", &CPU::OP_SBC, &CPU::ADR_ABS, 4},
            /* 0xEE */ {"INC", &CPU::OP_INC, &CPU::ADR_ABS, 6},
            /* 0xEF */ {"ISB", &CPU::OP_ISC, &CPU::ADR_ABS, 6}, // unofficial

            /* 0xFX */
            /* 0xF0 */ {"BEQ", &CPU::OP_BEQ, &CPU::ADR_REL, 2},
            /* 0xF1 */ {"SBC", &CPU::OP_SBC, &CPU::ADR_IZY, 5},
            /* 0xF2 */ {"JAM", &CPU::OP_NOP, &CPU::ADR_IMP, 0}, // illegal
            /* 0xF3 */ {"ISB", &CPU::OP_ISC, &CPU::ADR_IZY, 8}, // unofficial
            /* 0xF4 */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ZPX, 4},
            /* 0xF5 */ {"SBC", &CPU::OP_SBC, &CPU::ADR_ZPX, 4},
            /* 0xF6 */ {"INC", &CPU::OP_INC, &CPU::ADR_ZPX, 6},
            /* 0xF7 */ {"ISB", &CPU::OP_ISC, &CPU::ADR_ZPX, 6}, // unofficial
            /* 0xF8 */ {"SED", &CPU::OP_SED, &CPU::ADR_IMP, 2},
            /* 0xF9 */ {"SBC", &CPU::OP_SBC, &CPU::ADR_ABY, 4},
            /* 0xFA */ {"NOP", &CPU::OP_NOP, &CPU::ADR_IMP, 2},
            /* 0xFB */ {"ISB", &CPU::OP_ISC, &CPU::ADR_ABY, 7}, // unofficial
            /* 0xFC */ {"NOP", &CPU::OP_NOP, &CPU::ADR_ABX, 4},
            /* 0xFD */ {"SBC", &CPU::OP_SBC, &CPU::ADR_ABX, 4},
            /* 0xFE */ {"INC", &CPU::OP_INC, &CPU::ADR_ABX, 7},
            /* 0xFF */ {"ISB", &CPU::OP_ISC, &CPU::ADR_ABX, 7}, // unofficial
        }
    };

public:
    // =====================
    // == Getters/Setters ==
    // =====================
    [[nodiscard]] uint32_t TotalCycles() const {
        return total_cycles_;
    }

    [[nodiscard]] uint8_t A() const {
        return a_;
    }

    void set_a(const uint8_t a) {
        a_ = a;
    }

    [[nodiscard]] uint8_t X() const {
        return x_;
    }

    void set_x(const uint8_t x) {
        x_ = x;
    }

    [[nodiscard]] uint8_t Y() const {
        return y_;
    }

    void set_Y(const uint8_t y) {
        y_ = y;
    }

    [[nodiscard]] uint8_t SP() const {
        return sp_;
    }

    void set_SP(const uint8_t sp) {
        sp_ = sp;
    }

    [[nodiscard]] uint16_t PC() const {
        return pc_;
    }

    void set_PC(const uint16_t pc) {
        pc_ = pc;
    }

    [[nodiscard]] uint8_t P() const {
        return p_;
    }

    void set_P(const uint8_t p) {
        p_ = p;
    }

    [[nodiscard]] static Operation GetOpcodeEntry(const uint8_t opcode) {
        return kOpcodeTable[opcode];
    }

    // Flag manipulation functions
    void SetFlag(Flags flag, bool value);

    [[nodiscard]] bool GetFlag(Flags flag) const;

    // Bus link and reading/writing
    void Bus(Bus* bus_ptr) {
        bus_ = bus_ptr;
    }

    [[nodiscard]] bool IsComplete() const {
        return current_cycle_ == 0;
    }

    uint8_t Fetch() {
        if (current_addr_mode_ == &CPU::ADR_IMP) {
            return a_;
        }
        return Read(fetched_address_);
    }
};
