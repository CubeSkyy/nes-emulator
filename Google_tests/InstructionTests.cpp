#include "gtest/gtest.h"
#include "Nes.h"
#include "Rom.h"

class InstructionTests : public ::testing::Test {

protected:
    Nes nes;

    InstructionTests() {
        nes = Nes();
    }

    ~InstructionTests() override {
        // You can do clean-up work that doesn't throw exceptions here.
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following methods:

    void SetUp() override {
        nes.reset(true);
    }

    void TearDown() override {
    }
};

// LDA  ---------------------------------------------------------------------------------------------------------------
TEST_F(InstructionTests, LDA_IMM) {
    std::vector<uint8_t> chrRom = {0xA9, 0xAA};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xAA);
}

TEST_F(InstructionTests, LDA_ZP) {
    std::vector<uint8_t> chrRom = {0xA5, 0xA5};
    nes.rom.setChrRom(chrRom);
    nes.memory[0xA5] = 0xA7;
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xA7);
}

TEST_F(InstructionTests, LDA_ZP_IND_X) {
    uint8_t offset = 0x10;
    vector<uint8_t> chrRom = {0xB5, 0xA3};
    nes.rom.setChrRom(chrRom);
    nes.reg_X = offset;
    nes.memory[0xA3 + nes.reg_X] = 0xA8;
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xA8);
}

TEST_F(InstructionTests, LDA_ABS) {
    vector<uint8_t> chrRom = {0xAD, 0xBB, 0xCC};
    nes.rom.setChrRom(chrRom);
    nes.memory[0xBBCC] = 0xA2;
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xA2);
}

TEST_F(InstructionTests, LDA_ABS_X) {
    uint8_t offset = 0xDA;
    vector<uint8_t> chrRom = {0xBD, 0xB5, 0xC2};
    nes.rom.setChrRom(chrRom);
    nes.reg_X = offset;
    nes.memory[0xB5C2 + offset] = 0xA1;
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xA1);
}

TEST_F(InstructionTests, LDA_ABS_Y) {
    uint8_t offset = 0xDA;
    vector<uint8_t> chrRom = {0xB9, 0xB5, 0xC2};
    nes.rom.setChrRom(chrRom);
    nes.reg_Y = offset;
    nes.memory[0xB5C2 + offset] = 0xA1;
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xA1);
}

TEST_F(InstructionTests, LDA_IND_X) {
    uint8_t offset = 0xDA;
    vector<uint8_t> chrRom = {0xA1, 0xAC};
    nes.rom.setChrRom(chrRom);
    nes.reg_X = offset;
    nes.memory[0xAC + offset] = 0xA1;
    nes.memory[0xAC + offset + 0x04] = 0xA2;
    nes.memory[0xA2A1] = 0xFF;
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xFF);
}

TEST_F(InstructionTests, LDA_IND_Y) {
    uint8_t offset = 0xD1;
    vector<uint8_t> chrRom = {0xB1, 0xA2};
    nes.rom.setChrRom(chrRom);
    nes.reg_Y = offset;
    nes.memory[0xA2] = 0xA6;
    nes.memory[0xA2 + 0x04] = 0xA3;
    nes.memory[0xA3A6 + offset] = 0xF1;
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xF1);
}

TEST_F(InstructionTests, LDA_FLAGS) {
    std::vector<uint8_t> chrRom = {0xA9, 0x01};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getZeroFlag(), 0);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    nes.reset(true);

    chrRom = {0xA9, 0x00};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getZeroFlag(), 1);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    nes.reset(true);

    chrRom = {0xA9, 0xFF};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getZeroFlag(), 0);
    EXPECT_EQ(nes.getNegativeFlag(), 1);
}


// ADC ---------------------------------------------------------------------------------------------------------------
TEST_F(InstructionTests, ADC_NORMAL) {
    std::vector<uint8_t> chrRom = {0xA9, 0x19, 0x69, 0x19};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getZeroFlag(), 0);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getOverflowFlag(), 0);
    EXPECT_EQ(nes.reg_A, 0x32);
}

TEST_F(InstructionTests, ADC_NEGATIVE) {
    std::vector<uint8_t> chrRom = {0xA9, 0xF1, 0x69, 0x01};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.reg_A, 0xF2);
}

TEST_F(InstructionTests, ADC_OVERFLOW_NORMAL) {
    std::vector<uint8_t> chrRom = {0xA9, 0xFF, 0x69, 0x01};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getOverflowFlag(), 1);
    EXPECT_EQ(nes.reg_A, 0x00);
}

TEST_F(InstructionTests, ADC_DECIMAL_MODE) {
    std::vector<uint8_t> chrRom = {0xA9, 0x19, 0x69, 0x19};
    nes.rom.setChrRom(chrRom);
    nes.setDecimalModeFlag(1);
    nes.loop();
    EXPECT_EQ(nes.getOverflowFlag(), 0);
    EXPECT_EQ(nes.reg_A, 38);
}

TEST_F(InstructionTests, ADC_CARRY_BCD) {
    std::vector<uint8_t> chrRom = {0xA9, 0x99, 0x69, 0x01};
    nes.rom.setChrRom(chrRom);
    nes.setDecimalModeFlag(1);
    nes.loop();
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.reg_A, 100);
}


// ORA ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, ORA_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xF0, 0x09, 0x0F};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xFF);
}

TEST_F(InstructionTests, ORA_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x09, 0xC4};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xE5);
}


// AND ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, AND_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xFF, 0x29, 0xF0};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xF0);
}

TEST_F(InstructionTests, AND_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x29, 0xC4};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x84);
}

// EOR ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, EOR_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xF0, 0x49, 0x0F};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xFF);
}

TEST_F(InstructionTests, ERO_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x49, 0xC4};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x61);
}

// STA ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, STA) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x85, 0xA6};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.memory[0xA6], 0xA5);
}

// CMP ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, CMP_LESSER) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xC9, 0xA6};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.getCarryFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

TEST_F(InstructionTests, CMP_EQUAL) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xC9, 0xA5};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 1);
}

TEST_F(InstructionTests, CMP_GREATER) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xC9, 0xA4};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}


// SBC ---------------------------------------------------------------------------------------------------------------
// With carry
TEST_F(InstructionTests, SBC_HEX_WITH_CARRY_BEFORE_POS) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xE9, 0x02};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xA3);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

TEST_F(InstructionTests, SBC_HEX_WITH_CARRY_BEFORE_ZERO) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xE9, 0xA5};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x00);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 1);
}

TEST_F(InstructionTests, SBC_HEX_WITH_CARRY_BEFORE_NEG) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xE9, 0xA6};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xFF);
    EXPECT_EQ(nes.getCarryFlag(), 0);
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

// Without carry
TEST_F(InstructionTests, SBC_HEX_WITHOUT_CARRY_BEFORE_POS) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xE9, 0x02};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xA2);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

TEST_F(InstructionTests, SBC_HEX_WITHOUT_CARRY_BEFORE_ZERO) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xE9, 0xA4};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x00);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 1);
}


TEST_F(InstructionTests, SBC_HEX_WITHOUT_CARRY_BEFORE_NEG) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0xE9, 0xA5};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xFF);
    EXPECT_EQ(nes.getCarryFlag(), 0);
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

// ASL ---------------------------------------------------------------------------------------------------------------
// 1110 0000 -> 1|1100 0000
TEST_F(InstructionTests, ASL_CARRY_1_NEG) {
    std::vector<uint8_t> chrRom = {0xA9, 0xE0, 0x0A};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xC0);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getNegativeFlag(), 1);
}

// 0110 0000 -> 0|1100 0000
TEST_F(InstructionTests, ASL_CARRY_0_NEG) {
    std::vector<uint8_t> chrRom = {0xA9, 0x60, 0x0A};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xC0);
    EXPECT_EQ(nes.getCarryFlag(), 0);
    EXPECT_EQ(nes.getNegativeFlag(), 1);
}

// 1010 0000 -> 1|0100 0000
TEST_F(InstructionTests, ASL_CARRY_1_POS) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA0, 0x0A};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x40);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
}

// 0010 0000 -> 0|0100 0000
TEST_F(InstructionTests, ASL_CARRY_0_POS) {
    std::vector<uint8_t> chrRom = {0xA9, 0x20, 0x0A};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x40);
    EXPECT_EQ(nes.getCarryFlag(), 0);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
}


// ROL ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, ROL_WITHOUT_CARRY_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x2A};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x4A);
    EXPECT_EQ(nes.getCarryFlag(), 1);
}

TEST_F(InstructionTests, ROL_WITHOUT_CARRY_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0x4B, 0x2A};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x96);
    EXPECT_EQ(nes.getCarryFlag(), 0);
}

TEST_F(InstructionTests, ROL_WITH_CARRY_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x2A};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x4B);
    EXPECT_EQ(nes.getCarryFlag(), 1);
}

TEST_F(InstructionTests, ROL_WITH_CARRY_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0x4B, 0x2A};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x97);
    EXPECT_EQ(nes.getCarryFlag(), 0);
}


// LSR ---------------------------------------------------------------------------------------------------------------


TEST_F(InstructionTests, LSR_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x4A};//10100101
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x52);
    EXPECT_EQ(nes.getCarryFlag(), 1);
}


TEST_F(InstructionTests, LSR_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA4, 0x4A};//10100100
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x52);
    EXPECT_EQ(nes.getCarryFlag(), 0);
}


// ROR ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, ROR_WITHOUT_CARRY_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x6A}; //10100101
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x52);
    EXPECT_EQ(nes.getCarryFlag(), 1);
}

TEST_F(InstructionTests, ROR_WITHOUT_CARRY_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0x4A, 0x6A}; //01001010
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0x25);
    EXPECT_EQ(nes.getCarryFlag(), 0);
}

TEST_F(InstructionTests, ROR_WITH_CARRY_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xA5, 0x6A};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xD2);
    EXPECT_EQ(nes.getCarryFlag(), 1);
}

TEST_F(InstructionTests, ROR_WITH_CARRY_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0x4A, 0x6A};//01001010
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xA5);
    EXPECT_EQ(nes.getCarryFlag(), 0);
}

// BCC ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BCC_JUMP) {
    std::vector<uint8_t> chrRom = {0x90, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x03);
}

TEST_F(InstructionTests, BCC_NOJUMP) {
    std::vector<uint8_t> chrRom = {0x90, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x02);
}

// BCS ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BCS_JUMP) {
    std::vector<uint8_t> chrRom = {0xB0, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x03);
}

TEST_F(InstructionTests, BCS_NOJUMP) {
    std::vector<uint8_t> chrRom = {0xB0, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(0);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x02);
}

// BEQ ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BEQ_JUMP) {
    std::vector<uint8_t> chrRom = {0xF0, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setZeroFlag(1);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x03);
}

TEST_F(InstructionTests, BEQ_NOJUMP) {
    std::vector<uint8_t> chrRom = {0xF0, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setZeroFlag(0);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x02);
}


// BIT ---------------------------------------------------------------------------------------------------------------


TEST_F(InstructionTests, BIT_ZP_1) {
    std::vector<uint8_t> chrRom = {0xA9, 0xFF, 0x24, 0xA5}; //1111 1111
    nes.rom.setChrRom(chrRom);
    nes.memory[0xA5] = 0xFF; //1111 1111
    nes.setZeroFlag(1);
    nes.setNegativeFlag(0);
    nes.setOverflowFlag(0);
    nes.loop();
    EXPECT_EQ(nes.getZeroFlag(), 0);
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.getOverflowFlag(), 1);
}

TEST_F(InstructionTests, BIT_ZP_2) {
    std::vector<uint8_t> chrRom = {0xA9, 0xFF, 0x24, 0xA5}; //1111 1111
    nes.rom.setChrRom(chrRom);
    nes.memory[0xA5] = 0x00; //0000 0000
    nes.setZeroFlag(0);
    nes.setNegativeFlag(1);
    nes.setOverflowFlag(1);
    nes.loop();
    EXPECT_EQ(nes.getZeroFlag(), 1);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getOverflowFlag(), 0);
}

// BMI ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BMI_JUMP) {
    std::vector<uint8_t> chrRom = {0x30, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setNegativeFlag(1);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x03);
}

TEST_F(InstructionTests, BMI_NOJUMP) {
    std::vector<uint8_t> chrRom = {0x30, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setNegativeFlag(0);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x02);
}

// BNE ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BNE_JUMP) {
    std::vector<uint8_t> chrRom = {0xD0, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setZeroFlag(0);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x03);
}

TEST_F(InstructionTests, BNE_NOJUMP) {
    std::vector<uint8_t> chrRom = {0xD0, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setZeroFlag(1);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x02);
}

// BPL ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BPL_JUMP) {
    std::vector<uint8_t> chrRom = {0x10, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setNegativeFlag(0);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x03);
}

TEST_F(InstructionTests, BPL_NOJUMP) {
    std::vector<uint8_t> chrRom = {0x10, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setNegativeFlag(1);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x02);
}
// BRK  ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BRK) {
    std::vector<uint8_t> chrRom = {0x00};
    nes.rom.setChrRom(chrRom);

    // Set up initial status register (P)
    nes.reg_stat = 0b11000000;
    nes.memory[0xFFFF] = 0x02;
    nes.memory[0xFFFE] = 0x03;
    nes.loop();

    // Check the stack contents
    EXPECT_EQ(nes.pullFromStack(), 0b11010000);  // Status register (P) with bit 4 (Break flag) set
    EXPECT_EQ(nes.pullFromStack(), 0x01);  // pc.l
    EXPECT_EQ(nes.pullFromStack(), 0x00);  // pc.h

    // Load the interrupt vector from $FFFE and $FFFF
    EXPECT_EQ(nes.pc, 0x0203 + 1);
}

// BVC ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BVC_JUMP) {
    std::vector<uint8_t> chrRom = {0x50, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setOverflowFlag(0);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x03);
}

TEST_F(InstructionTests, BVC_NOJUMP) {
    std::vector<uint8_t> chrRom = {0x50, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setOverflowFlag(1);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x02);
}

// BVS ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, BVS_JUMP) {
    std::vector<uint8_t> chrRom = {0x70, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setOverflowFlag(1);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x03);
}

TEST_F(InstructionTests, BVS_NOJUMP) {
    std::vector<uint8_t> chrRom = {0x70, 0x03};
    nes.rom.setChrRom(chrRom);
    nes.setOverflowFlag(0);
    nes.loop();
    EXPECT_EQ(nes.pc, 0x02);
}

// CLC ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, CLC) {
    std::vector<uint8_t> chrRom = {0x18};
    nes.rom.setChrRom(chrRom);
    nes.setCarryFlag(1);
    nes.loop();
    EXPECT_EQ(nes.getCarryFlag(), 0);
}

// CLD ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, CLD) {
    std::vector<uint8_t> chrRom = {0xD8};
    nes.rom.setChrRom(chrRom);
    nes.setDecimalModeFlag(1);
    nes.loop();
    EXPECT_EQ(nes.getDecimalModeFlag(), 0);
}

// CLI ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, CLI) {
    std::vector<uint8_t> chrRom = {0x58};
    nes.rom.setChrRom(chrRom);
    nes.setInterruptFlag(1);
    nes.loop();
    EXPECT_EQ(nes.getInterruptFlag(), 0);
}

// CLV ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, CLV) {
    std::vector<uint8_t> chrRom = {0xB8};
    nes.rom.setChrRom(chrRom);
    nes.setOverflowFlag(1);
    nes.loop();
    EXPECT_EQ(nes.getOverflowFlag(), 0);
}


// LDX  ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, LDX_IMM) {
    std::vector<uint8_t> chrRom = {0xA2, 0xAA};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_X, 0xAA);
}

TEST_F(InstructionTests, LDX_ZP) {
    std::vector<uint8_t> chrRom = {0xA6, 0xA5};
    nes.rom.setChrRom(chrRom);
    nes.memory[0xA5] = 0xA7;
    nes.loop();
    EXPECT_EQ(nes.reg_X, 0xA7);
}

TEST_F(InstructionTests, LDX_ZP_IND_Y) {
    vector<uint8_t> chrRom = {0xB6, 0xA3};
    nes.rom.setChrRom(chrRom);
    nes.reg_Y = 0x10;
    nes.memory[0xA3 + nes.reg_Y] = 0xA8;
    nes.loop();
    EXPECT_EQ(nes.reg_X, 0xA8);
}

TEST_F(InstructionTests, LDX_ABS) {
    vector<uint8_t> chrRom = {0xAE, 0xBB, 0xCC};
    nes.rom.setChrRom(chrRom);
    nes.memory[0xBBCC] = 0xA2;
    nes.loop();
    EXPECT_EQ(nes.reg_X, 0xA2);
}

TEST_F(InstructionTests, LDX_ABS_Y) {
    vector<uint8_t> chrRom = {0xBE, 0xB5, 0xC2};
    nes.rom.setChrRom(chrRom);
    nes.reg_Y = 0xDA;
    nes.memory[0xB5C2 + nes.reg_Y] = 0xA1;
    nes.loop();
    EXPECT_EQ(nes.reg_X, 0xA1);
}

// LDY  ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, LDY_IMM) {
    std::vector<uint8_t> chrRom = {0xA0, 0xAA};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_Y, 0xAA);
}

TEST_F(InstructionTests, LDY_ZP) {
    std::vector<uint8_t> chrRom = {0xA4, 0xA5};
    nes.rom.setChrRom(chrRom);
    nes.memory[0xA5] = 0xA7;
    nes.loop();
    EXPECT_EQ(nes.reg_Y, 0xA7);
}

TEST_F(InstructionTests, LDY_ZP_IND_X) {
    vector<uint8_t> chrRom = {0xB4, 0xA3};
    nes.rom.setChrRom(chrRom);
    nes.reg_X = 0x10;
    nes.memory[0xA3 + nes.reg_X] = 0xA8;
    nes.loop();
    EXPECT_EQ(nes.reg_Y, 0xA8);
}

TEST_F(InstructionTests, LDY_ABS) {
    vector<uint8_t> chrRom = {0xAC, 0xBB, 0xCC};
    nes.rom.setChrRom(chrRom);
    nes.memory[0xBBCC] = 0xA2;
    nes.loop();
    EXPECT_EQ(nes.reg_Y, 0xA2);
}

TEST_F(InstructionTests, LDY_ABS_X) {
    vector<uint8_t> chrRom = {0xBC, 0xB5, 0xC2};
    nes.rom.setChrRom(chrRom);
    nes.reg_X = 0xDA;
    nes.memory[0xB5C2 + nes.reg_X] = 0xA1;
    nes.loop();
    EXPECT_EQ(nes.reg_Y, 0xA1);
}

// CPX ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, CPX_LESSER) {
    std::vector<uint8_t> chrRom = {0xA2, 0xA5, 0xE0, 0xA6};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.getCarryFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

TEST_F(InstructionTests, CPX_EQUAL) {
    std::vector<uint8_t> chrRom = {0xA2, 0xA5, 0xE0, 0xA5};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 1);
}

TEST_F(InstructionTests, CPX_GREATER) {
    std::vector<uint8_t> chrRom = {0xA2, 0xA5, 0xE0, 0xA4};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}
// CPY ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, CPY_LESSER) {
    std::vector<uint8_t> chrRom = {0xA0, 0xA5, 0xC0, 0xA6};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 1);
    EXPECT_EQ(nes.getCarryFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

TEST_F(InstructionTests, CPY_EQUAL) {
    std::vector<uint8_t> chrRom = {0xA0, 0xA5, 0xC0, 0xA5};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 1);
}

TEST_F(InstructionTests, CPY_GREATER) {
    std::vector<uint8_t> chrRom = {0xA0, 0xA5, 0xC0, 0xA4};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getCarryFlag(), 1);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

// DEC ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, DEC_ABS) {
    std::vector<uint8_t> chrRom = {0xCE, 0xAA, 0xAA};
    nes.rom.setChrRom(chrRom);
    nes.memory[0xAAAA] = 10;
    nes.loop();
    EXPECT_EQ(nes.memory[0xAAAA], 9);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

// DEX ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, DEX) {
    std::vector<uint8_t> chrRom = {0xA2, 10, 0xCA};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_X, 9);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

// DEY ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, DEY) {
    std::vector<uint8_t> chrRom = {0xA0, 10, 0x88};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_Y, 9);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}


// INC ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, INC_ABS) {
    std::vector<uint8_t> chrRom = {0xEE, 0xAA, 0xAA};
    nes.rom.setChrRom(chrRom);
    nes.memory[0xAAAA] = 10;
    nes.loop();
    EXPECT_EQ(nes.memory[0xAAAA], 11);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}


// INX ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, INX) {
    std::vector<uint8_t> chrRom = {0xA2, 10, 0xE8};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_X, 11);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}

// INY ---------------------------------------------------------------------------------------------------------------

TEST_F(InstructionTests, INY) {
    std::vector<uint8_t> chrRom = {0xA0, 10, 0xC8};
    nes.rom.setChrRom(chrRom);
    nes.loop();
    EXPECT_EQ(nes.reg_Y, 11);
    EXPECT_EQ(nes.getNegativeFlag(), 0);
    EXPECT_EQ(nes.getZeroFlag(), 0);
}
