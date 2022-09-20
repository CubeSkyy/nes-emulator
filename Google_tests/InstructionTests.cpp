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
        // Code here will be called immediately after each test (right
        // before the destructor).
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

// TODO: Finish LDA tests
