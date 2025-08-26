//
// Created by CubeSky on 16/06/2025.
//

#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include <iostream>
#include <queue>
#include <set>

#include "log/logging.h"

Bus::Bus(CPU* cpu, PPU* ppu) : cpu_(cpu), ppu_(ppu) {
    cpu->Bus(this);
    ram_.fill(0);
    cpu->Reset();
}

Bus::~Bus() = default;

void Bus::Step() {
    ppu_->Step();
    if (total_cycles_ % 3 == 0) {
        // Perform DMA read/write every 2 cycles
        if (dma_active_ && dma_dummy_ && total_cycles_ % 2 == 1) {
            // dma_dummy is used to wait 1 cycle before stating transfer if the current cycle is even
            // This guarantees that the transfer starts on a read
            dma_dummy_ = false;
        }
        else if (dma_active_ && !dma_dummy_ && total_cycles_ % 2 == 0) { // Read after sync
            dma_data_ = Read(256 * dma_page_ + dma_addr_);
        }
        else if (dma_active_ && !dma_dummy_ && total_cycles_ % 2 == 1) {
            ppu_->oam_.bytes[dma_addr_] = dma_data_;
            dma_addr_++;
            if (dma_addr_ == 0) { // Detect DMA end with overflow
                dma_active_ = false;
                dma_dummy_ = true;
            }
        }else { // No dma, normal step
            cpu_->Step();
        }
    }

    if (ppu_->was_nmi_triggered_) {
        cpu_->NMI();
        ppu_->was_nmi_triggered_ = false;
    }

    total_cycles_++;
}


bool Bus::LoadCartridge(const std::string& filename) {
    // Create a new cartridge object
    cartridge_ = std::make_shared<Cartridge>(filename);

    if (!cartridge_->isLoaded()) {
        std::cerr << "Failed to load ROM: " << filename << std::endl;
        return false;
    }
    // Reset CPU state
    cpu_->Reset();
    ppu_->cartridge_ = cartridge_;

    GenerateDisassembly();
    std::cout << "ROM loaded successfully: " << filename << std::endl;
    return true;
}

bool Bus::InitEmptyCartridge() {
    // Initialize an empty cartridge for unit testing
    cartridge_ = std::make_shared<Cartridge>();
    if (!cartridge_->isLoaded()) {
        std::cerr << "Failed to initialize empty cartridge." << std::endl;
        return false;
    }
    // Reset CPU state
    cpu_->Reset();

    std::cout << "Initialized empty cartridge for testing." << std::endl;
    return true;
}


uint8_t Bus::Read(const uint16_t address) {
    // CPU RAM with 2Kb mirroring
    if (address >= 0x0000 && address <= 0x1FFF) {
        return ram_[address & 0x7FF];
    }
    else if (address >= 0x2000 && address <= 0x3FFF) {
        return ppu_->CpuRead(address & 0x0007); // PPU registers are mirrored every 8 bytes
    }
    else if (address >= 0x4016 && address <= 0x4017) {
        // Controller input handling
        uint8_t data = (controller_shift_reg[address - 0x4016] & 0b10000000) > 0;
        controller_shift_reg[address - 0x4016] <<= 1;
        return data;
    }
    else if (address >= 0x6000 && address <= 0xFFFF && cartridge_) {
        return cartridge_->CpuRead(address);
    }
    return 0x00;
}

void Bus::Write(const uint16_t address, const uint8_t value) {
    if (address >= 0x0000 && address <= 0x1FFF) {
        ram_[address & 0x7FF] = value;
    }
    else if (address >= 0x2000 && address <= 0x3FFF) {
        ppu_->CpuWrite(address & 0x0007, value); // PPU registers are mirrored every 8 bytes
    }
    else if (address == 0x4014) {
        // DMA transfer
        dma_active_ = true;
        dma_page_ = value;
        dma_addr_ = 0x00;
    }
    else if (address >= 0x4016 && address <= 0x4017) {
        // Controller input handling
        controller_shift_reg[address - 0x4016] = curr_controller_state[address - 0x4016];
    }
    else if (address >= 0x6000 && address <= 0xFFFF && cartridge_) {
        cartridge_->CpuWrite(address, value);
    }
}

void Bus::GenerateDisassembly() {
    disassembly_.clear();
    if (cartridge_ == nullptr || cartridge_->prg_rom_.empty()) return;

    std::set<uint16_t> visited;
    std::queue<uint16_t> to_process;
    to_process.push(cartridge_->reset_vector_);

    while (!to_process.empty()) {
        uint16_t addr = to_process.front();
        to_process.pop();
        if (visited.count(addr)) continue;
        visited.insert(addr);

        std::string line = Logging::Trim(Logging::CreateDisassemblyLine(*cpu_, addr));
        disassembly_[addr] = line;
        uint8_t opcode = Read(addr);
        auto entry = CPU::GetOpcodeEntry(opcode);
        uint8_t size = Logging::GetOperationLength(entry.addr_mode_);
        if (size == 0) size = 1;
        uint16_t next_addr = static_cast<uint16_t>(addr + size);

        // Handle control flow
        std::string opname = entry.name_;
        if (opname == "JMP") {
            // Absolute or indirect jump
            if (entry.addr_mode_ == &CPU::ADR_ABS) {
                uint16_t target = Read(addr + 1) | (Read(addr + 2) << 8);
                to_process.push(target);
            }
            else if (entry.addr_mode_ == &CPU::ADR_IND) {
                uint16_t ptr = Read(addr + 1) | (Read(addr + 2) << 8);
                uint8_t lo = Read(ptr);
                uint8_t hi = Read((ptr & 0xFF00) | ((ptr + 1) & 0xFF));
                uint16_t target = (hi << 8) | lo;
                to_process.push(target);
            }
            continue; // Do not follow next_addr
        }
        else if (opname == "JSR") {
            // Subroutine call: follow both target and next_addr
            uint16_t target = Read(addr + 1) | (Read(addr + 2) << 8);
            to_process.push(target);
            to_process.push(next_addr);
        }
        else if (entry.addr_mode_ == &CPU::ADR_REL && (
            opname == "BPL" || opname == "BMI" || opname == "BVC" || opname == "BVS" ||
            opname == "BCC" || opname == "BCS" || opname == "BNE" || opname == "BEQ")) {
            // Branch: follow both branch target and next_addr
            int8_t offset = static_cast<int8_t>(Read(addr + 1));
            uint16_t branch_target = static_cast<uint16_t>(addr + 2 + offset);
            to_process.push(branch_target);
            to_process.push(next_addr);
        }
        else if (opname == "RTS" || opname == "RTI" || opname == "BRK") {
            // End of flow
            continue;
        }
        else {
            // Normal instruction: follow next_addr
            to_process.push(next_addr);
        }
    }
}

void Bus::DoDMA(uint8_t page) {
    // DMA takes 513 or 514 cycles to complete
    // 1 idle cycle at the start, then 256 alternating read/write cycles
    // If on an odd CPU cycle, add 1 extra idle cycle
    dma_active_ = true;
    dma_addr_ = static_cast<uint16_t>(page) << 8;

    if (total_cycles_ % 2 == 1) {
        dma_dummy_ = 1; // Need an extra cycle if starting on odd cycle
    }
}
