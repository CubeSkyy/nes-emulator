#include "cpu.h"

#include <fstream>
#include <iostream>


#include "bus.h"

// =====================
// === Public API ======
// =====================
CPU::CPU() {
    Reset();
#ifdef LOGMODE
    // If file exists, clear it, if it doesn't, create it
    std::ofstream neslog_file("neslog_output.log", std::fstream::trunc);
#endif
}


// CPU memory access
uint8_t CPU::Read(const uint16_t addr) const {
    return bus_->Read(addr);
}

void CPU::Write(const uint16_t addr, const uint8_t data) const {
    bus_->Write(addr, data);
}

void CPU::Step() {
    if (current_cycle_ == 0) {
#ifdef LOGMODE
        Logging::create_neslog_line(*this);
#endif

        const uint8_t opcode = Read(pc_++);

        current_operation_ = kOpcodeTable[opcode];
        current_addr_mode_ = kOpcodeTable[opcode].addr_mode_;
        current_cycle_ = kOpcodeTable[opcode].cycles_;

        (this->*kOpcodeTable[opcode].addr_mode_)();
        (this->*kOpcodeTable[opcode].op_function_)();
    }
    total_cycles_++;
    current_cycle_--;
}

void CPU::StepInstruction() {
    do {
        Step();
    }
    while (!IsComplete());
}

void CPU::Reset() {
    a_ = 0x00;
    x_ = 0x00;
    y_ = 0x00;
    sp_ = 0xFD;

    if (bus_)
        pc_ = (Read(0xFFFC) | (Read(0xFFFD) << 8)); // Set PC from reset vector

    if (pc_ == 0x0000 || bus_ == nullptr)
        pc_ = 0x6000;

    p_ = 0x00;
    SetFlag(R, true);
    SetFlag(I, true);
    current_cycle_ = 7;
    total_cycles_ = 0; // Reset cycle count
    fetched_address_ = 0x0000;
}

// Interrupts
void CPU::IRQ() {
    if (GetFlag(I) == 0) {
        // Only respond to IRQ if interrupts are enabled
        // Push the program counter to the stack
        Write(0x100 + sp_--, (pc_ >> 8) & 0x00FF); // Push high byte
        Write(0x100 + sp_--, pc_ & 0x00FF); // Push low byte

        // Push the processor status to the stack
        SetFlag(B, false);
        SetFlag(R, true);
        SetFlag(I, true);
        Write(0x100 + sp_--, p_ | 0x20);

        // Jump to the interrupt vector
        pc_ = Read(0xFFFE) | (Read(0xFFFF) << 8);
        current_cycle_ = 7; // IRQ takes 7 cycles to process
    }
}

void CPU::NMI() {
    // Push the program counter to the stack
    Write(0x100 + sp_--, (pc_ >> 8) & 0x00FF); // Push high byte
    Write(0x100 + sp_--, pc_ & 0x00FF); // Push low byte

    // Push the processor status to the stack
    SetFlag(B, false);
    SetFlag(R, true);
    SetFlag(I, true);
    Write(0x100 + sp_--, p_);

    // Jump to the NMI vector
    pc_ = Read(0xFFFA) | (Read(0xFFFB) << 8);
    current_cycle_ = 8; // NMI takes 8 cycles to process
}

void CPU::RTI() {
    // Pull the processor status from the stack
    p_ = Read(0x100 + ++sp_); // Ignore bit break bit (4)
    SetFlag(B, false);
    SetFlag(R, true);


    // Pull the program counter from the stack
    const uint8_t lo = Read(0x100 + ++sp_);
    const uint8_t hi = Read(0x100 + ++sp_);
    pc_ = (hi << 8) | lo;
}


// Addressing Modes
void CPU::ADR_IMP() {
    fetched_address_ = 0;
    // For implicit addressing, Fetch will return A, skip here
}

void CPU::ADR_IMM() {
    fetched_address_ = pc_++;
}

void CPU::ADR_ZP0() {
    fetched_address_ = Read(pc_++) & 0x00FF;
}

void CPU::ADR_ZPX() {
    fetched_address_ = (Read(pc_++) + x_) & 0x00FF;
}

void CPU::ADR_ZPY() {
    fetched_address_ = (Read(pc_++) + y_) & 0x00FF;
}

void CPU::ADR_ABS() {
    const uint8_t lo = Read(pc_++);
    const uint8_t hi = Read(pc_++);
    fetched_address_ = (hi << 8) | lo;
}

void CPU::ADR_ABX() {
    const uint8_t lo = Read(pc_++);
    const uint8_t hi = Read(pc_++);
    fetched_address_ = ((hi << 8) | lo) + x_;

    if ((fetched_address_ & 0xFF00) != (hi << 8) &&
        kStoreOps.find(current_operation_.name_) == kStoreOps.end()) {
        current_cycle_++; // Only for non-store ops
    }
}

void CPU::ADR_ABY() {
    const uint8_t lo = Read(pc_++);
    const uint8_t hi = Read(pc_++);
    fetched_address_ = ((hi << 8) | lo) + y_;

    if ((fetched_address_ & 0xFF00) != (hi << 8) &&
        kStoreOps.find(current_operation_.name_) == kStoreOps.end()) {
        current_cycle_++; // Only for non-store ops
    }
}

void CPU::ADR_IND() {
    const uint8_t ptr_lo = Read(pc_++);
    const uint8_t ptr_hi = Read(pc_++);
    const uint16_t ptr = (ptr_hi << 8) | ptr_lo;

    if (ptr_lo == 0x00FF)
        fetched_address_ = (Read(ptr & 0xFF00) << 8) | Read(ptr);
    else
        fetched_address_ = (Read(ptr + 1) << 8) | Read(ptr);
}

void CPU::ADR_IZX() {
    const uint8_t base = Read(pc_++);
    const uint8_t lo = Read((base + x_) & 0x00FF);
    const uint8_t hi = Read((base + x_ + 1) & 0x00FF);
    fetched_address_ = (hi << 8) | lo;
}

void CPU::ADR_IZY() {
    const uint8_t base = Read(pc_++);
    const uint8_t lo = Read(base & 0x00FF);
    const uint8_t hi = Read((base + 1) & 0x00FF);
    fetched_address_ = (hi << 8) | lo;
    fetched_address_ += y_;

    if ((fetched_address_ & 0xFF00) != (hi << 8) &&
        kStoreOps.find(current_operation_.name_) == kStoreOps.end()) {
        current_cycle_++; // Only for non-store ops
    }
}

void CPU::ADR_REL() {
    const auto offset = static_cast<int8_t>(Read(pc_++));
    fetched_address_ = pc_ + offset;
}


// Operations
void CPU::OP_ADC() {
    const uint8_t value = Fetch();
    const uint16_t result = a_ + value + GetFlag(C);


    SetFlag(C, result > 0xFF); // Set carry flag if result exceeds 8 bits
    SetFlag(Z, (result & 0x00FF) == 0x00); // Set zero flag if result is zero
    SetFlag(V, ~(a_ ^ value) & (a_ ^ result) & 0x80); // Set overflow flag if the addition results in an overflow
    SetFlag(N, result & 0x80);

    a_ = result & 0x00FF;
}

void CPU::OP_AND() {
    a_ &= Fetch();
    SetFlag(Z, a_ == 0x00);
    SetFlag(N, a_ & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_ASL() {
    // Shift A left and set carry flag if bit 7 is set

    const uint16_t temp = Fetch() << 1;
    SetFlag(C, (temp & 0xFF00) > 0);

    if (current_addr_mode_ == &CPU::ADR_IMP) {
        a_ = temp & 0x00FF; // If the addressing mode is implicit, store the result in A
    }
    else {
        Write(fetched_address_, temp & 0x00FF); // Otherwise, write the result back to memory
    }
    // Set zero and negative flags based on the result
    SetFlag(Z, (temp & 0x00FF) == 0x00);
    SetFlag(N, temp & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_BCC() {
    if (GetFlag(C) == 0) {
        current_cycle_++; // Branches taken use an extra cycle
        if ((fetched_address_ & 0xFF00) != (pc_ & 0xFF00))
            current_cycle_++; // Extra cycle if crossing page boundary

        pc_ = fetched_address_;
    }
}

void CPU::OP_BCS() {
    if (GetFlag(C) == 1) {
        current_cycle_++; // Branches taken use an extra cycle
        if ((fetched_address_ & 0xFF00) != (pc_ & 0xFF00))
            current_cycle_++; // Extra cycle if crossing page boundary

        pc_ = fetched_address_;
    }
}

void CPU::OP_BEQ() {
    if (GetFlag(Z) == 1) {
        current_cycle_++; // Branch taken
        if ((fetched_address_ & 0xFF00) != (pc_ & 0xFF00))
            current_cycle_++; // Extra cycle if crossing page boundary
        pc_ = fetched_address_;
    }
}


void CPU::OP_BIT() {
    const uint8_t value = Fetch();
    SetFlag(Z, (a_ & value) == 0); // Set zero flag if A AND value is zero
    SetFlag(N, value & 0x80); // Set negative flag if bit 7 of value is set
    SetFlag(V, value & 0x40); // Set overflow flag if bit 6 of value is set
}

void CPU::OP_BMI() {
    if (GetFlag(N) == 1) {
        current_cycle_++; // Branches taken use an extra cycle
        if ((fetched_address_ & 0xFF00) != (pc_ & 0xFF00))
            current_cycle_++; // Extra cycle if crossing page boundary

        pc_ = fetched_address_;
    }
}

void CPU::OP_BNE() {
    if (GetFlag(Z) == 0) {
        current_cycle_++; // Branches taken use an extra cycle
        if ((fetched_address_ & 0xFF00) != (pc_ & 0xFF00))
            current_cycle_++; // Extra cycle if crossing page boundary

        pc_ = fetched_address_;
    }
}

void CPU::OP_BPL() {
    if (GetFlag(N) == 0) {
        current_cycle_++; // Branches taken use an extra cycle
        if ((fetched_address_ & 0xFF00) != (pc_ & 0xFF00))
            current_cycle_++; // Extra cycle if crossing page boundary

        pc_ = fetched_address_;
    }
}

void CPU::OP_BRK() {
    // Push the program counter to the stack
    SetFlag(I, true);
    Write(0x100 + sp_--, (pc_ >> 8) & 0xFF); // Push high byte
    Write(0x100 + sp_--, pc_ & 0xFF); // Push low byte


    // Push the processor status to the stack
    SetFlag(B, true);
    Write(0x100 + sp_--, p_);
    SetFlag(B, false);

    // Jump to the interrupt vector
    pc_ = (Read(0xFFFE) | (Read(0xFFFF) << 8));
}

void CPU::OP_BVC() {
    if (GetFlag(V) == 0) {
        current_cycle_++; // Branches taken use an extra cycle
        if ((fetched_address_ & 0xFF00) != (pc_ & 0xFF00))
            current_cycle_++; // Extra cycle if crossing page boundary

        pc_ = fetched_address_;
    }
}

void CPU::OP_BVS() {
    if (GetFlag(V) == 1) {
        current_cycle_++; // Branches taken use an extra cycle
        if ((fetched_address_ & 0xFF00) != (pc_ & 0xFF00))
            current_cycle_++; // Extra cycle if crossing page boundary

        pc_ = fetched_address_;
    }
}

void CPU::OP_CLC() {
    SetFlag(C, false);
}

void CPU::OP_CLD() {
    SetFlag(D, false);
}

void CPU::OP_CLI() {
    SetFlag(I, false);
}

void CPU::OP_CLV() {
    SetFlag(V, false);
}

void CPU::OP_CMP() {
    const uint8_t value = Fetch();
    const uint8_t result = a_ - value;

    // Set carry flag if A >= value
    SetFlag(C, a_ >= value);

    // Set zero flag if A == value
    SetFlag(Z, a_ == value);

    // Set negative flag based on the result
    SetFlag(N, result & 0x80);
}

void CPU::OP_CPX() {
    const uint8_t value = Fetch();
    const uint8_t result = x_ - value;

    // Set carry flag if X >= value
    SetFlag(C, x_ >= value);

    // Set zero flag if X == value
    SetFlag(Z, x_ == value);

    // Set negative flag based on the result
    SetFlag(N, result & 0x80);
}

void CPU::OP_CPY() {
    const uint8_t value = Fetch();
    const uint8_t result = y_ - value;

    // Set carry flag if Y >= value
    SetFlag(C, y_ >= value);

    // Set zero flag if Y == value
    SetFlag(Z, y_ == value);

    // Set negative flag based on the result
    SetFlag(N, result & 0x80);
}

void CPU::OP_DEC() {
    // Decrement the value at the fetched address
    const uint8_t value = Fetch() - 1;
    Write(fetched_address_, value);

    // Set zero and negative flags based on the decremented value
    SetFlag(Z, value == 0);
    SetFlag(N, value & 0x80);
}

void CPU::OP_DEX() {
    x_--;
    SetFlag(Z, x_ == 0);
    SetFlag(N, x_ & 0x80);
}

void CPU::OP_DEY() {
    y_--;
    SetFlag(Z, y_ == 0);
    SetFlag(N, y_ & 0x80);
}

void CPU::OP_EOR() {
    a_ ^= Fetch();
    SetFlag(Z, a_ == 0x00);
    SetFlag(N, a_ & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_INC() {
    // Increment the value at the fetched address
    const uint8_t value = Fetch() + 1;
    Write(fetched_address_, value);

    // Set zero and negative flags based on the incremented value
    SetFlag(Z, value == 0);
    SetFlag(N, value & 0x80);
}

void CPU::OP_INX() {
    x_++;
    SetFlag(Z, x_ == 0);
    SetFlag(N, x_ & 0x80);
}

void CPU::OP_INY() {
    y_++;
    SetFlag(Z, y_ == 0);
    SetFlag(N, y_ & 0x80);
}

void CPU::OP_JMP() {
    pc_ = fetched_address_; // Jump to the address
}

void CPU::OP_JSR() {
    // Push the return address to the stack
    Write(0x100 + sp_--, (pc_ - 1 >> 8) & 0xFF); // Push high byte
    Write(0x100 + sp_--, pc_ - 1 & 0xFF); // Push low byte

    // Set the program counter to the target address
    pc_ = fetched_address_;
}

void CPU::OP_LDA() {
    // Load the accumulator with the value at the fetched address
    a_ = Fetch();

    // Set zero and negative flags based on A
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_LDX() {
    // Load the X register with the value at the fetched address
    x_ = Fetch();

    // Set zero and negative flags based on X
    SetFlag(Z, x_ == 0);
    SetFlag(N, x_ & 0x80); // Set negative flag if X is negative (bit 7 is set)
}

void CPU::OP_LDY() {
    // Load the Y register with the value at the fetched address
    y_ = Fetch();

    // Set zero and negative flags based on Y
    SetFlag(Z, y_ == 0);
    SetFlag(N, y_ & 0x80); // Set negative flag if Y is negative (bit 7 is set)
}

void CPU::OP_LSR() {
    // Logical Shift Right
    const uint8_t value = Fetch();
    SetFlag(C, value & 0x01); // Set carry flag if bit 0 is set
    const uint8_t temp = value >> 1; // Shift A right by 1

    if (current_addr_mode_ == &CPU::ADR_IMP) {
        a_ = temp; // If the addressing mode is implicit, store the result in A
    }
    else {
        Write(fetched_address_, temp); // Otherwise, write the result back to memory
    }
    // Set zero and negative flags based on A
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_NOP() {
    // No operation, just a placeholder
}

void CPU::OP_ORA() {
    a_ |= Fetch(); // Bitwise OR with the accumulator
    SetFlag(Z, a_ == 0x00); // Set zero flag if A is zero
    SetFlag(N, a_ & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_PHA() {
    // Push the accumulator onto the stack
    Write(0x100 + sp_--, a_);
}

void CPU::OP_PHP() {
    // Push the processor status onto the stack
    SetFlag(B, true);
    Write(0x100 + sp_--, p_);
    SetFlag(B, false);
}

void CPU::OP_PLA() {
    // Pull the accumulator from the stack
    a_ = Read(0x100 + ++sp_);

    // Set zero and negative flags based on A
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_PLP() {
    // Read new status from stack
    p_ = Read(0x100 + ++sp_);
    SetFlag(B, false);
    SetFlag(R, true);
}

void CPU::OP_ROL() {
    // Rotate Left
    const uint8_t value = Fetch();
    const bool carry = GetFlag(C);
    SetFlag(C, value & 0x80); // Set carry flag if bit 7 is set
    const uint8_t temp = (value << 1) | (carry ? 1 : 0); // Shift A left and set bit 0 to previous carry

    if (current_addr_mode_ == &CPU::ADR_IMP) {
        a_ = temp; // If the addressing mode is implicit, store the result in A
    }
    else {
        Write(fetched_address_, temp); // Otherwise, write the result back to memory
    }
    // Set zero and negative flags based on A
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_ROR() {
    // Rotate Right
    const uint8_t value = Fetch();
    const bool carry = GetFlag(C);
    SetFlag(C, value & 0x01); // Set carry flag if bit 0 is set
    const uint8_t temp = value >> 1 | (carry ? 0x80 : 0); // Shift A right and set bit 7 to previous carry

    if (current_addr_mode_ == &CPU::ADR_IMP) {
        a_ = temp; // If the addressing mode is implicit, store the result in A
    }
    else {
        Write(fetched_address_, temp); // Otherwise, write the result back to memory
    }
    // Set zero and negative flags based on A
    SetFlag(Z, temp == 0);
    SetFlag(N, temp & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_RTS() {
    // Pull the program counter from the stack
    const uint8_t lo = Read(0x100 + ++sp_);
    const uint8_t hi = Read(0x100 + ++sp_);
    pc_ = (hi << 8) | lo;

    // Increment PC to point to the next instruction
    pc_++;
}

void CPU::OP_SBC() {
    const uint8_t value = Fetch();

    // Invert the value and use ADC operation
    // SBC is essentially ADC with the ones complement of the operand
    const uint16_t result = a_ + (~value & 0xFF) + GetFlag(C);

    SetFlag(C, result > 0xFF); // Set carry flag if result exceeds 8 bits
    SetFlag(Z, (result & 0x00FF) == 0x00); // Set zero flag if result is zero
    SetFlag(V, (a_ ^ value) & (a_ ^ result) & 0x80); // Set overflow flag if the addition results in an overflow
    SetFlag(N, result & 0x80);

    a_ = result & 0x00FF;
}

void CPU::OP_SEC() {
    SetFlag(C, true);
}

void CPU::OP_SED() {
    SetFlag(D, true);
}

void CPU::OP_SEI() {
    SetFlag(I, true);
}

void CPU::OP_STA() {
    // Store the accumulator at the fetched address
    Write(fetched_address_, a_);
}

void CPU::OP_STX() {
    // Store the X register at the fetched address
    Write(fetched_address_, x_);
}

void CPU::OP_STY() {
    // Store the Y register at the fetched address
    Write(fetched_address_, y_);
}

void CPU::OP_TAX() {
    // Transfer the accumulator to the X register
    x_ = a_;

    // Set zero and negative flags based on X
    SetFlag(Z, x_ == 0);
    SetFlag(N, x_ & 0x80); // Set negative flag if X is negative (bit 7 is set)
}

void CPU::OP_TAY() {
    // Transfer the accumulator to the Y register
    y_ = a_;

    // Set zero and negative flags based on Y
    SetFlag(Z, y_ == 0);
    SetFlag(N, y_ & 0x80); // Set negative flag if Y is negative (bit 7 is set)
}

void CPU::OP_TSX() {
    // Transfer the stack pointer to the X register
    x_ = sp_;

    // Set zero and negative flags based on X
    SetFlag(Z, x_ == 0);
    SetFlag(N, x_ & 0x80); // Set negative flag if X is negative (bit 7 is set)
}

void CPU::OP_TXA() {
    // Transfer the X register to the accumulator
    a_ = x_;

    // Set zero and negative flags based on A
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

void CPU::OP_TXS() {
    // Transfer the X register to the stack pointer
    sp_ = x_;

    // Note: TXS does not affect any flags
}

void CPU::OP_TYA() {
    // Transfer the Y register to the accumulator
    a_ = y_;

    // Set zero and negative flags based on A
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80); // Set negative flag if A is negative (bit 7 is set)
}

// LAX: Load A and X with memory (unofficial)
void CPU::OP_LAX() {
    const uint8_t value = Fetch();
    a_ = value;
    x_ = value;
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
}

// SAX: Store A & X (unofficial)
void CPU::OP_SAX() {
    Write(fetched_address_, a_ & x_);
}

// DCP: DEC then CMP (unofficial)
void CPU::OP_DCP() {
    // Decrement memory
    const uint8_t value = Fetch() - 1;
    Write(fetched_address_, value);

    // Compare with accumulator
    const uint8_t result = a_ - value;
    SetFlag(C, a_ >= value);
    SetFlag(Z, a_ == value);
    SetFlag(N, result & 0x80);
}

// SLO: ASL memory, then ORA with accumulator (unofficial)
void CPU::OP_SLO() {
    // Perform ASL on memory
    uint8_t value = Fetch();
    SetFlag(C, value & 0x80); // Set carry if bit 7 was set
    value <<= 1;
    Write(fetched_address_, value);

    // ORA with accumulator
    a_ |= value;
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
}

void CPU::OP_ANC() {
    a_ &= Fetch();
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
    SetFlag(C, a_ & 0x80); // Carry = Negative
}

void CPU::OP_RLA() {
    // Rotate memory left
    uint8_t value = Fetch();
    const bool old_carry = GetFlag(C);
    SetFlag(C, value & 0x80);
    value = (value << 1) | (old_carry ? 1 : 0);
    Write(fetched_address_, value);

    // AND with accumulator
    a_ &= value;
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
}

void CPU::OP_SRE() {
    // LSR memory
    uint8_t value = Fetch();
    SetFlag(C, value & 0x01); // Carry = old bit 0
    value >>= 1;
    Write(fetched_address_, value);

    // EOR with accumulator
    a_ ^= value;
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
}

// RRA: ROR memory, then ADC (unofficial)
void CPU::OP_RRA() {
    uint8_t value = Fetch();
    const bool old_carry = GetFlag(C);
    SetFlag(C, value & 0x01);
    value = (value >> 1) | (old_carry ? 0x80 : 0);
    Write(fetched_address_, value);

    // ADC with rotated value
    const uint16_t result = a_ + value + GetFlag(C);
    SetFlag(C, result > 0xFF);
    SetFlag(Z, (result & 0xFF) == 0);
    SetFlag(V, (~(a_ ^ value) & (a_ ^ result) & 0x80));
    SetFlag(N, result & 0x80);
    a_ = result & 0xFF;
}

// XAA: A = X & immediate (unofficial, highly unstable)
void CPU::OP_XAA() {
    a_ = x_ & Fetch();
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
}

// TAS: (a.k.a. SHS) S = A & X, store (A & X) & (hi+1) at address (unofficial)
void CPU::OP_TAS() {
    sp_ = a_ & x_;
    const uint8_t value = sp_ & ((fetched_address_ >> 8) + 1);
    Write(fetched_address_, value);
}

// SHY: Store Y & (hi+1) at address (unofficial)
void CPU::OP_SHY() {
    const uint8_t value = y_ & ((fetched_address_ >> 8) + 1);
    Write(fetched_address_, value);
}

// SHX: Store X & (hi+1) at address (unofficial)
void CPU::OP_SHX() {
    const uint8_t value = x_ & ((fetched_address_ >> 8) + 1);
    Write(fetched_address_, value);
}

// LXA: A = X = A & immediate (unofficial, unstable)
void CPU::OP_LXA() {
    a_ = x_ = (a_ & Fetch());
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
}

// LAS: A, X, S = memory & S (unofficial)
void CPU::OP_LAS() {
    const uint8_t value = Fetch() & sp_;
    a_ = x_ = sp_ = value;
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
}

// AXS: X = (A & X) - immediate (unofficial)
void CPU::OP_AXS() {
    const uint8_t orig_value = Fetch();
    const uint8_t value = (a_ & x_) - orig_value;
    SetFlag(C, (a_ & x_) >= orig_value);
    SetFlag(Z, value == 0);
    SetFlag(N, value & 0x80);
    x_ = value;
}

// ISC: INC memory, then SBC (unofficial)
void CPU::OP_ISC() {
    const uint8_t value = Fetch() + 1;
    Write(fetched_address_, value);

    // SBC with incremented value
    const uint16_t result = a_ + (~value & 0xFF) + GetFlag(C);
    SetFlag(C, result > 0xFF);
    SetFlag(Z, (result & 0xFF) == 0);
    SetFlag(V, (a_ ^ value) & (a_ ^ result) & 0x80);
    SetFlag(N, result & 0x80);
    a_ = result & 0xFF;
}

void CPU::OP_ARR() {
    a_ &= Fetch();
    a_ = (a_ >> 1) | (GetFlag(C) ? 0x80 : 0);
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
    SetFlag(C, a_ & 0x40);
    SetFlag(V, ((a_ & 0x40) >> 6) ^ ((a_ & 0x20) >> 5));
}

void CPU::OP_ASR() {
    // ASR: AND with immediate, then LSR A
    a_ &= Fetch();
    SetFlag(C, a_ & 0x01);
    a_ >>= 1;
    SetFlag(Z, a_ == 0);
    SetFlag(N, a_ & 0x80);
}

void CPU::OP_UNF() {
    // Unimplemented unofficial opcode
    std::cerr << "Unimplemented opcode encountered at PC: " << std::hex << pc_ - 1 << std::dec << std::endl;
}

void CPU::SetFlag(const Flags flag, const bool value) {
    if (value)
        p_ |= (1 << flag);
    else
        p_ &= ~(1 << flag);
}

bool CPU::GetFlag(const Flags flag) const {
    return (p_ >> flag) & 1;
}
