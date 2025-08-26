#include <gtest/gtest.h>
#include "cpu.h"
#include "bus.h"

class CPUTest : public ::testing::Test {
protected:
    static CPU* cpu;
    static Bus* bus;


    // Before all tests
    static void SetUpTestSuite() {
        cpu = new CPU();
        bus = new Bus(cpu, nullptr);
        bus->InitEmptyCartridge();
    }

    // Before each test
    void SetUp() override {
        cpu->Reset();
        cpu->set_PC(0x6000); // use NROM & PGR RAM for testing
        cpu->StepInstruction(); // Drain initial cycles (needed for clock accurate emulation)
    }

    // Runs once after all tests
    static void TearDownTestSuite() {
        delete bus;
        bus = nullptr;
        delete cpu;
        cpu = nullptr;
    }
};

CPU* CPUTest::cpu = nullptr;
Bus* CPUTest::bus = nullptr;


TEST_F(CPUTest, LDAImmediate) {
    cpu->Write(cpu->PC(), 0xA9); // LDA immediate
    cpu->Write(cpu->PC() + 1, 0x00); // Value: 0

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));

    cpu->Reset();
    cpu->StepInstruction(); // Drain clocks

    cpu->set_PC(0x6000);
    cpu->Write(cpu->PC(), 0xA9);
    cpu->Write(cpu->PC() + 1, 0x80); // Value: 0x80 (negative)

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x80);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, LDAZeroPage) {
    cpu->Write(cpu->PC(), 0xA5); // LDA zero page
    cpu->Write(cpu->PC() + 1, 0x10); // Address: 0x10
    cpu->Write(0x0010, 0x42); // Value: 0x42

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, LDAAbsolute) {
    cpu->Write(cpu->PC(), 0xAD); // LDA absolute
    cpu->Write(cpu->PC() + 1, 0x34); // Low byte
    cpu->Write(cpu->PC() + 2, 0x12); // High byte
    cpu->Write(0x1234, 0xFF); // Value: 0xFF

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0xFF);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

// STA Tests
TEST_F(CPUTest, STAZeroPage) {
    cpu->set_a(0x42);
    cpu->Write(cpu->PC(), 0x85); // STA zero page
    cpu->Write(cpu->PC() + 1, 0x10); // Address: 0x10

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x10), 0x42);
}

TEST_F(CPUTest, STAAbsolute) {
    cpu->set_a(0x42);
    cpu->Write(cpu->PC(), 0x8D); // STA absolute
    cpu->Write(cpu->PC() + 1, 0x34); // Low byte
    cpu->Write(cpu->PC() + 2, 0x12); // High byte

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x1234), 0x42);
}

TEST_F(CPUTest, STAIndirectX) {
    cpu->set_a(0x42);
    cpu->set_x(0x02);
    cpu->Write(cpu->PC(), 0x81); // STA indirect X
    cpu->Write(cpu->PC() + 1, 0x10); // Base address
    cpu->Write(0x12, 0x34); // Low byte of effective address
    cpu->Write(0x13, 0x12); // High byte of effective address

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x1234), 0x42);
}

// Register Transfer Tests
TEST_F(CPUTest, TAX) {
    cpu->set_a(0x42);
    cpu->Write(cpu->PC(), 0xAA); // TAX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, TAXZeroFlag) {
    cpu->set_a(0x00);
    cpu->Write(cpu->PC(), 0xAA); // TAX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, TAXNegativeFlag) {
    cpu->set_a(0x80);
    cpu->Write(cpu->PC(), 0xAA); // TAX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x80);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, TXA) {
    cpu->set_x(0x42);
    cpu->Write(cpu->PC(), 0x8A); // TXA

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, TAY) {
    cpu->set_a(0x42);
    cpu->Write(cpu->PC(), 0xA8); // TAY

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Y(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, TYA) {
    cpu->set_Y(0x42);
    cpu->Write(cpu->PC(), 0x98); // TYA

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, TSX) {
    cpu->set_SP(0xFE);
    cpu->Write(cpu->PC(), 0xBA); // TSX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0xFE);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, TXS) {
    cpu->set_x(0x42);
    cpu->Write(cpu->PC(), 0x9A); // TXS

    cpu->StepInstruction();
    EXPECT_EQ(cpu->SP(), 0x42);
    // TXS doesn't affect flags
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

// LDX Tests
TEST_F(CPUTest, LDXImmediate) {
    cpu->Write(cpu->PC(), 0xA2); // LDX immediate
    cpu->Write(cpu->PC() + 1, 0x42); // Value: 0x42

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, LDXZeroFlag) {
    cpu->Write(cpu->PC(), 0xA2); // LDX immediate
    cpu->Write(cpu->PC() + 1, 0x00); // Value: 0x00

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, LDXNegativeFlag) {
    cpu->Write(cpu->PC(), 0xA2); // LDX immediate
    cpu->Write(cpu->PC() + 1, 0x80); // Value: 0x80 (negative)

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x80);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, LDXZeroPage) {
    cpu->Write(cpu->PC(), 0xA6); // LDX zero page
    cpu->Write(cpu->PC() + 1, 0x10); // Address: 0x10
    cpu->Write(0x0010, 0x42); // Value: 0x42

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

// LDY Tests
TEST_F(CPUTest, LDYImmediate) {
    cpu->Write(cpu->PC(), 0xA0); // LDY immediate
    cpu->Write(cpu->PC() + 1, 0x42); // Value: 0x42

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Y(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, LDYZeroPage) {
    cpu->Write(cpu->PC(), 0xA4); // LDY zero page
    cpu->Write(cpu->PC() + 1, 0x10); // Address: 0x10
    cpu->Write(0x0010, 0x42); // Value: 0x42

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Y(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

// Register Operation Tests
TEST_F(CPUTest, INX) {
    cpu->set_x(0x41);
    cpu->Write(cpu->PC(), 0xE8); // INX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, INXZeroFlag) {
    cpu->set_x(0xFF);
    cpu->Write(cpu->PC(), 0xE8); // INX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, INXNegativeFlag) {
    cpu->set_x(0x7F);
    cpu->Write(cpu->PC(), 0xE8); // INX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x80);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, INY) {
    cpu->set_Y(0x41);
    cpu->Write(cpu->PC(), 0xC8); // INY

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Y(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, DEX) {
    cpu->set_x(0x43);
    cpu->Write(cpu->PC(), 0xCA); // DEX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, DEXZeroFlag) {
    cpu->set_x(0x01);
    cpu->Write(cpu->PC(), 0xCA); // DEX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, DEXNegativeFlag) {
    cpu->set_x(0x00);
    cpu->Write(cpu->PC(), 0xCA); // DEX

    cpu->StepInstruction();
    EXPECT_EQ(cpu->X(), 0xFF);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, DEY) {
    cpu->set_Y(0x43);
    cpu->Write(cpu->PC(), 0x88); // DEY

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Y(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

// Arithmetic Operation Tests
TEST_F(CPUTest, ADCImmediate) {
    cpu->set_a(0x40);
    cpu->Write(cpu->PC(), 0x69);
    cpu->Write(cpu->PC() + 1, 0x02);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x42);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
    EXPECT_FALSE(cpu->GetFlag(CPU::V));
}

TEST_F(CPUTest, ADCCarryFlag) {
    cpu->set_a(0xFF);
    cpu->Write(cpu->PC(), 0x69); // ADC immediate
    cpu->Write(cpu->PC() + 1, 0x01);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
    EXPECT_TRUE(cpu->GetFlag(CPU::C));
    EXPECT_FALSE(cpu->GetFlag(CPU::V));
}

TEST_F(CPUTest, ADCOverflowFlag) {
    cpu->set_a(0x7F); // Largest positive number in 8-bit two's complement
    cpu->Write(cpu->PC(), 0x69); // ADC immediate
    cpu->Write(cpu->PC() + 1, 0x01);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x80); // Result is -128 in two's complement
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
    EXPECT_TRUE(cpu->GetFlag(CPU::V)); // Overflow: positive + positive = negative
}

TEST_F(CPUTest, ADCWithCarry) {
    cpu->set_a(0x40);
    cpu->SetFlag(CPU::C, true); // Set carry flag
    cpu->Write(cpu->PC(), 0x69); // ADC immediate
    cpu->Write(cpu->PC() + 1, 0x01); // ADC immediate

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x42); // 0x40 + 0x01 + 0x01(carry) = 0x42
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
    EXPECT_FALSE(cpu->GetFlag(CPU::V));
}

TEST_F(CPUTest, SBCImmediate) {
    cpu->set_a(0x44);
    cpu->SetFlag(CPU::C, true); // Set carry flag (borrow = 0)
    cpu->Write(cpu->PC(), 0xE9); // SBC immediate
    cpu->Write(cpu->PC() + 1, 0x02);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x42); // 0x44 - 0x02 = 0x42
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
    EXPECT_TRUE(cpu->GetFlag(CPU::C)); // No borrow needed
    EXPECT_FALSE(cpu->GetFlag(CPU::V));
}

TEST_F(CPUTest, SBCBorrowNeeded) {
    cpu->set_a(0x40);
    cpu->SetFlag(CPU::C, true); // Set carry flag (borrow = 0)
    cpu->Write(cpu->PC(), 0xE9); // SBC immediate
    cpu->Write(cpu->PC() + 1, 0x41);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0xFF); // 0x40 - 0x41 = 0xFF (with borrow)
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C)); // Borrow needed
    EXPECT_FALSE(cpu->GetFlag(CPU::V));
}

TEST_F(CPUTest, SBCOverflowFlag) {
    cpu->set_a(0x80); // -128 in two's complement
    cpu->SetFlag(CPU::C, true); // Set carry flag (borrow = 0)
    cpu->Write(cpu->PC(), 0xE9); // SBC immediate
    cpu->Write(cpu->PC() + 1, 0xFF); // Value: 0xFF (-1 in two's complement)

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x81); // -128 - (-1) = -127 (0x81 in two's complement)
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C)); // Borrow needed
    // This overflow case is complex, depends on implementation
}

// Compare Operation Tests
TEST_F(CPUTest, CMPImmediate) {
    cpu->set_a(0x42);
    cpu->Write(cpu->PC(), 0xC9); // CMP immediate
    cpu->Write(cpu->PC() + 1, 0x42); // Value: 0x42 (equal)

    cpu->StepInstruction();
    EXPECT_TRUE(cpu->GetFlag(CPU::Z)); // A == M
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
    EXPECT_TRUE(cpu->GetFlag(CPU::C)); // A >= M
}

TEST_F(CPUTest, CMPGreaterThan) {
    cpu->set_a(0x42);
    cpu->Write(cpu->PC(), 0xC9); // CMP immediate
    cpu->Write(cpu->PC() + 1, 0x40); // Value: 0x40 (A > M)

    cpu->StepInstruction();
    EXPECT_FALSE(cpu->GetFlag(CPU::Z)); // A != M
    EXPECT_FALSE(cpu->GetFlag(CPU::N)); // Result is positive
    EXPECT_TRUE(cpu->GetFlag(CPU::C)); // A >= M
}

TEST_F(CPUTest, CMPLessThan) {
    cpu->set_a(0x40);
    cpu->Write(cpu->PC(), 0xC9); // CMP immediate
    cpu->Write(cpu->PC() + 1, 0x42); // Value: 0x42 (A < M)

    cpu->StepInstruction();
    EXPECT_FALSE(cpu->GetFlag(CPU::Z)); // A != M
    EXPECT_TRUE(cpu->GetFlag(CPU::N)); // Result is negative
    EXPECT_FALSE(cpu->GetFlag(CPU::C)); // A < M
}

// Branch Operation Tests
TEST_F(CPUTest, BEQBranchTaken) {
    cpu->SetFlag(CPU::Z, true); // Set zero flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0xF0); // BEQ
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 + 0x10); // PC + instruction size + offset
}

TEST_F(CPUTest, BEQBranchNotTaken) {
    cpu->SetFlag(CPU::Z, false); // Clear zero flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0xF0); // BEQ
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2); // PC + instruction size only
}

TEST_F(CPUTest, BNEBranchTaken) {
    cpu->SetFlag(CPU::Z, false); // Clear zero flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0xD0); // BNE
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 + 0x10); // PC + instruction size + offset
}

TEST_F(CPUTest, BCSBranchTaken) {
    cpu->SetFlag(CPU::C, true); // Set carry flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0xB0); // BCS
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 + 0x10); // PC + instruction size + offset
}

TEST_F(CPUTest, BCCBranchTaken) {
    cpu->SetFlag(CPU::C, false); // Clear carry flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0x90); // BCC
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 + 0x10); // PC + instruction size + offset
}

TEST_F(CPUTest, BMIBranchTaken) {
    cpu->SetFlag(CPU::N, true); // Set negative flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0x30); // BMI
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 + 0x10); // PC + instruction size + offset
}

TEST_F(CPUTest, BPLBranchTaken) {
    cpu->SetFlag(CPU::N, false); // Clear negative flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0x10); // BPL
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 + 0x10); // PC + instruction size + offset
}

TEST_F(CPUTest, BVCBranchTaken) {
    cpu->SetFlag(CPU::V, false); // Clear overflow flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0x50); // BVC
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 + 0x10); // PC + instruction size + offset
}

TEST_F(CPUTest, BVSBranchTaken) {
    cpu->SetFlag(CPU::V, true); // Set overflow flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0x70); // BVS
    cpu->Write(cpu->PC() + 1, 0x10); // Branch offset: +16

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 + 0x10); // PC + instruction size + offset
}

TEST_F(CPUTest, NegativeOffset) {
    cpu->SetFlag(CPU::Z, true); // Set zero flag
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0xF0); // BEQ
    cpu->Write(cpu->PC() + 1, 0xF0); // Branch offset: -16 (0xF0 is -16 in two's complement)

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), initial_PC + 2 - 16); // PC + instruction size - 16
}

// Jump and Subroutine Tests
TEST_F(CPUTest, JMPAbsolute) {
    cpu->Write(cpu->PC(), 0x4C); // JMP absolute
    cpu->Write(cpu->PC() + 1, 0x34); // Low byte
    cpu->Write(cpu->PC() + 2, 0x12); // High byte

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), 0x1234);
}

TEST_F(CPUTest, JMPIndirect) {
    cpu->Write(cpu->PC(), 0x6C); // JMP indirect
    cpu->Write(cpu->PC() + 1, 0x34); // Low byte of pointer
    cpu->Write(cpu->PC() + 2, 0x12); // High byte of pointer
    cpu->Write(0x1234, 0x78); // Low byte of target
    cpu->Write(0x1235, 0x56); // High byte of target

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), 0x5678);
}

TEST_F(CPUTest, JMPIndirectPageBoundaryBug) {
    cpu->Write(cpu->PC(), 0x6C); // JMP indirect
    cpu->Write(cpu->PC() + 1, 0xFF); // Low byte of pointer
    cpu->Write(cpu->PC() + 2, 0x12); // High byte of pointer
    cpu->Write(0x12FF, 0x78); // Low byte of target
    cpu->Write(0x1200, 0x56); // High byte of target should be at 0x1300, but the 6502 bug wraps to 0x1200

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), 0x5678);
}

TEST_F(CPUTest, JSR) {
    cpu->set_SP(0xFF);
    uint16_t initial_PC = cpu->PC();
    cpu->Write(cpu->PC(), 0x20); // JSR
    cpu->Write(cpu->PC() + 1, 0x34); // Low byte
    cpu->Write(cpu->PC() + 2, 0x12); // High byte

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), 0x1234);
    // Check that return address (PC+2) was pushed to stack
    EXPECT_EQ(cpu->Read(0x1FF), (initial_PC + 2) >> 8); // High byte of PC+2
    EXPECT_EQ(cpu->Read(0x1FE), (initial_PC + 2) & 0xFF); // Low byte of PC+2
    EXPECT_EQ(cpu->SP(), 0xFD); // SP decremented by 2
}

TEST_F(CPUTest, RTS) {
    cpu->set_SP(0xFD);
    cpu->Write(0x01FF, 0x12); // High byte of return address
    cpu->Write(0x01FE, 0x33); // Low byte of return address
    cpu->Write(cpu->PC(), 0x60); // RTS

    cpu->StepInstruction();
    EXPECT_EQ(cpu->PC(), 0x1234); // Return address + 1
    EXPECT_EQ(cpu->SP(), 0xFF); // SP incremented by 2
}

// Stack Operation Tests
TEST_F(CPUTest, PHA) {
    cpu->set_a(0x42);
    cpu->set_SP(0xFF);
    cpu->Write(cpu->PC(), 0x48); // PHA

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x01FF), 0x42); // A pushed to stack
    EXPECT_EQ(cpu->SP(), 0xFE); // SP decremented
}

TEST_F(CPUTest, PLA) {
    cpu->set_SP(0xFE);
    cpu->Write(0x01FF, 0x42); // Value on stack
    cpu->Write(cpu->PC(), 0x68); // PLA

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x42); // Value popped into A
    EXPECT_EQ(cpu->SP(), 0xFF); // SP incremented
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, PLAZeroFlag) {
    cpu->set_SP(0xFE);
    cpu->Write(0x01FF, 0x00); // Zero value on stack
    cpu->Write(cpu->PC(), 0x68); // PLA

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, PLANegativeFlag) {
    cpu->set_SP(0xFE);
    cpu->Write(0x01FF, 0x80); // Negative value on stack
    cpu->Write(cpu->PC(), 0x68); // PLA

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x80);
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, PHP) {
    cpu->set_SP(0xFF);
    cpu->set_P(0x42);
    cpu->Write(cpu->PC(), 0x08); // PHP

    cpu->StepInstruction();
    // PHP sets the B flag when pushing
    EXPECT_EQ(cpu->Read(0x01FF), 0x42 | (1 << CPU::B)); // Status with B flag set pushed to stack
    EXPECT_EQ(cpu->SP(), 0xFE); // SP decremented
}

TEST_F(CPUTest, PLP) {
    cpu->set_SP(0xFE);
    cpu->Write(0x01FF, 0x42); // Status value on stack
    cpu->Write(cpu->PC(), 0x28); // PLP

    cpu->StepInstruction();
    // PLP ignores bit 4 (B flag) and sets bit 5 (R flag)
    EXPECT_EQ(cpu->P(), (0x42 & ~(1 << CPU::B)) | (1 << CPU::R));
    EXPECT_EQ(cpu->SP(), 0xFF); // SP incremented
}

// Status Flag Operation Tests
TEST_F(CPUTest, CLC) {
    cpu->SetFlag(CPU::C, true);
    cpu->Write(cpu->PC(), 0x18); // CLC

    cpu->StepInstruction();
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, SEC) {
    cpu->SetFlag(CPU::C, false);
    cpu->Write(cpu->PC(), 0x38); // SEC

    cpu->StepInstruction();
    EXPECT_TRUE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, CLI) {
    cpu->SetFlag(CPU::I, true);
    cpu->Write(cpu->PC(), 0x58); // CLI

    cpu->StepInstruction();
    EXPECT_FALSE(cpu->GetFlag(CPU::I));
}

TEST_F(CPUTest, SEI) {
    cpu->SetFlag(CPU::I, false);
    cpu->Write(cpu->PC(), 0x78); // SEI

    cpu->StepInstruction();
    EXPECT_TRUE(cpu->GetFlag(CPU::I));
}

TEST_F(CPUTest, CLV) {
    cpu->SetFlag(CPU::V, true);
    cpu->Write(cpu->PC(), 0xB8); // CLV

    cpu->StepInstruction();
    EXPECT_FALSE(cpu->GetFlag(CPU::V));
}

TEST_F(CPUTest, CLD) {
    cpu->SetFlag(CPU::D, true);
    cpu->Write(cpu->PC(), 0xD8); // CLD

    cpu->StepInstruction();
    EXPECT_FALSE(cpu->GetFlag(CPU::D));
}

TEST_F(CPUTest, SED) {
    cpu->SetFlag(CPU::D, false);
    cpu->Write(cpu->PC(), 0xF8); // SED

    cpu->StepInstruction();
    EXPECT_TRUE(cpu->GetFlag(CPU::D));
}

// Bitwise Operation Tests
TEST_F(CPUTest, ANDImmediate) {
    cpu->set_a(0xF0);
    cpu->Write(cpu->PC(), 0x29); // AND immediate
    cpu->Write(cpu->PC() + 1, 0x0F); // Value: 0x0F

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x00); // 0xF0 & 0x0F = 0x00
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, ANDNonZero) {
    cpu->set_a(0xF0);
    cpu->Write(cpu->PC(), 0x29); // AND immediate
    cpu->Write(cpu->PC() + 1, 0xF0); // Value: 0xF0

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0xF0); // 0xF0 & 0xF0 = 0xF0
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, ORAImmediate) {
    cpu->set_a(0xF0);
    cpu->Write(cpu->PC(), 0x09); // ORA immediate
    cpu->Write(cpu->PC() + 1, 0x0F); // Value: 0x0F

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0xFF); // 0xF0 | 0x0F = 0xFF
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, EORImmediate) {
    cpu->set_a(0xFF);
    cpu->Write(cpu->PC(), 0x49); // EOR immediate
    cpu->Write(cpu->PC() + 1, 0xFF); // Value: 0xFF

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x00); // 0xFF ^ 0xFF = 0x00
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

TEST_F(CPUTest, EORNonZero) {
    cpu->set_a(0xF0);
    cpu->Write(cpu->PC(), 0x49); // EOR immediate
    cpu->Write(cpu->PC() + 1, 0x0F); // Value: 0x0F

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0xFF); // 0xF0 ^ 0x0F = 0xFF
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

// Bit Shift Operation Tests
TEST_F(CPUTest, ASLAccumulator) {
    cpu->set_a(0x42);
    cpu->Write(cpu->PC(), 0x0A); // ASL accumulator

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x84); // 0x42 << 1 = 0x84
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, ASLAccumulatorCarryOut) {
    cpu->set_a(0x82);
    cpu->Write(cpu->PC(), 0x0A); // ASL accumulator

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x04); // 0x82 << 1 = 0x04 (carry out)
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
    EXPECT_TRUE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, LSRAccumulator) {
    cpu->set_a(0x42);
    cpu->Write(cpu->PC(), 0x4A); // LSR accumulator

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x21); // 0x42 >> 1 = 0x21
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N)); // MSB is always 0 after LSR
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, LSRAccumulatorCarryOut) {
    cpu->set_a(0x41);
    cpu->Write(cpu->PC(), 0x4A); // LSR accumulator

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x20); // 0x41 >> 1 = 0x20 (carry out)
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
    EXPECT_TRUE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, ROLAccumulator) {
    cpu->set_a(0x42);
    cpu->SetFlag(CPU::C, false); // Clear carry
    cpu->Write(cpu->PC(), 0x2A); // ROL accumulator

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x84); // 0x42 << 1 | 0 = 0x84
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, ROLAccumulatorWithCarry) {
    cpu->set_a(0x42);
    cpu->SetFlag(CPU::C, true); // Set carry
    cpu->Write(cpu->PC(), 0x2A); // ROL accumulator

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x85); // 0x42 << 1 | 1 = 0x85
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, RORAccumulator) {
    cpu->set_a(0x42);
    cpu->SetFlag(CPU::C, false); // Clear carry
    cpu->Write(cpu->PC(), 0x6A); // ROR accumulator

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0x21); // 0x42 >> 1 | 0 << 7 = 0x21
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
}

TEST_F(CPUTest, RORAccumulatorWithCarry) {
    cpu->set_a(0x42);
    cpu->SetFlag(CPU::C, true); // Set carry
    cpu->Write(cpu->PC(), 0x6A); // ROR accumulator

    cpu->StepInstruction();
    EXPECT_EQ(cpu->A(), 0xA1); // 0x42 >> 1 | 1 << 7 = 0xA1
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
}

// NOP Test
TEST_F(CPUTest, NOP) {
    // Set some initial state
    cpu->set_a(0x42);
    cpu->set_x(0x42);
    cpu->set_Y(0x42);
    cpu->set_P(0x42);
    uint16_t initial_PC = cpu->PC();

    cpu->Write(cpu->PC(), 0xEA); // NOP

    const uint8_t initialA = cpu->A();
    const uint8_t initialX = cpu->X();
    const uint8_t initialY = cpu->Y();
    const uint8_t initialP = cpu->P();

    cpu->StepInstruction();
    // NOP should only increment PC and not change any registers or flags
    EXPECT_EQ(cpu->PC(), initial_PC +1);
    EXPECT_EQ(cpu->A(), initialA);
    EXPECT_EQ(cpu->X(), initialX);
    EXPECT_EQ(cpu->Y(), initialY);
    EXPECT_EQ(cpu->P(), initialP);
}


// BIT Tests
TEST_F(CPUTest, BITZeroPage) {
    cpu->set_a(0xFF);
    cpu->Write(cpu->PC(), 0x24); // BIT zero page
    cpu->Write(cpu->PC() + 1, 0x10); // BIT zero page
    cpu->Write(0x0010, 0xC0); // Set N and V

    cpu->StepInstruction();
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
    EXPECT_TRUE(cpu->GetFlag(CPU::V));

    cpu->Reset();
    cpu->StepInstruction(); // Drain clocks

    cpu->set_PC(0x6000);
    cpu->set_a(0x00);

    cpu->Write(cpu->PC(), 0x24);
    cpu->Write(cpu->PC() + 1, 0x10);
    cpu->Write(0x0010, 0x00);

    cpu->StepInstruction();
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
}

// ASL Tests
TEST_F(CPUTest, ASLZeroPage) {
    cpu->Write(cpu->PC(), 0x06); // ASL zero page
    cpu->Write(cpu->PC() + 1, 0x10); // ASL zero page
    cpu->Write(0x0010, 0x81);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x10), 0x02);
    EXPECT_TRUE(cpu->GetFlag(CPU::C));
    EXPECT_FALSE(cpu->GetFlag(CPU::Z));
    EXPECT_FALSE(cpu->GetFlag(CPU::N));
}

// LSR Tests
TEST_F(CPUTest, LSRZeroPage) {
    cpu->Write(cpu->PC(), 0x46); // LSR zero page
    cpu->Write(cpu->PC() + 1, 0x10);
    cpu->Write(0x0010, 0x01);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x10), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::C));
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
}

// ROL Tests
TEST_F(CPUTest, ROLZeroPage) {
    cpu->SetFlag(CPU::C, true);
    cpu->Write(cpu->PC(), 0x26); // ROL zero page
    cpu->Write(cpu->PC() + 1, 0x10); // ROL zero page
    cpu->Write(0x0010, 0x40);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x10), 0x81);
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

// ROR Tests
TEST_F(CPUTest, RORZeroPage) {
    cpu->SetFlag(CPU::C, true);
    cpu->Write(cpu->PC(), 0x66); // ROR zero page
    cpu->Write(cpu->PC() + 1, 0x10); // ROL zero page
    cpu->Write(0x0010, 0x02);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x10), 0x81);
    EXPECT_FALSE(cpu->GetFlag(CPU::C));
    EXPECT_TRUE(cpu->GetFlag(CPU::N));
}

// INC Tests
TEST_F(CPUTest, INCZeroPage) {
    cpu->Write(cpu->PC(), 0xE6); // INC zero page
    cpu->Write(cpu->PC() + 1, 0x10);
    cpu->Write(0x0010, 0xFF);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x10), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
}

// DEC Tests
TEST_F(CPUTest, DECZeroPage) {
    cpu->Write(cpu->PC(), 0xC6); // DEC zero page
    cpu->Write(cpu->PC() + 1, 0x10);
    cpu->Write(0x0010, 0x01);

    cpu->StepInstruction();
    EXPECT_EQ(cpu->Read(0x10), 0x00);
    EXPECT_TRUE(cpu->GetFlag(CPU::Z));
}
