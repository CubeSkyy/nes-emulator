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
    }

    void TearDown() override {
        nes.reset(true);
    }
};


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
    nes.memory[0xA3+nes.reg_X] = 0xA8;

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

    nes.memory[0xAC+offset] = 0xA1;
    nes.memory[0xAC+offset+0x04] = 0xA2;
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
    nes.memory[0xA2+0x04] = 0xA3;
    nes.memory[0xA3A6+offset] = 0xF1;

    nes.loop();
    EXPECT_EQ(nes.reg_A, 0xF1);
}