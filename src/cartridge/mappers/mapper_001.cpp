#include "mapper_001.h"

Mapper001::Mapper001(const uint8_t prg_chunks)
    : prg_chunks_(prg_chunks) {
}

void Mapper001::WriteRegister(const uint16_t address, const uint8_t data) {
    // If bit 7 is set, reset shift register and control
    if (data & 0x80) {
        shift_register_ = 0x10;
        control_ |= 0x0C;
        write_count_ = 0;
        return;
    }

    // Shift in one bit from data (bit 0)
    shift_register_ = (shift_register_ >> 1) | ((data & 1) << 4);
    write_count_++;
    if (write_count_ == 5) {
        switch ((address >> 13) & 0x03) {
        case 0: control_ = shift_register_;
            break;
        case 1: chr_bank_0_ = shift_register_;
            break;
        case 2: chr_bank_1_ = shift_register_;
            break;
        case 3: prg_bank_ = shift_register_;
            break;
        default: ;
        }
        shift_register_ = 0x10;
        write_count_ = 0;
    }
}

uint32_t Mapper001::MapPrgAddress(const uint16_t cpu_addr) const {
    // PRG ROM banking logic
    const uint8_t prg_mode = (control_ >> 2) & 0x03;
    const uint32_t prg_bank_count = prg_chunks_;
    uint32_t bank = prg_bank_ & 0x0F;
    if (prg_mode == 0 || prg_mode == 1) {
        // 32KB mode
        bank &= ~1;
        return (bank * 0x4000) + (cpu_addr & 0x7FFF);
    }
    else if (prg_mode == 2) {
        // Fix first bank at $8000, switch 16KB at $C000
        if (cpu_addr < 0xC000)
            return cpu_addr & 0x3FFF;
        else
            return (bank * 0x4000) + (cpu_addr & 0x3FFF);
    }
    else {
        // Fix last bank at $C000, switch 16KB at $8000
        if (cpu_addr < 0xC000)
            return (bank * 0x4000) + (cpu_addr & 0x3FFF);
        else
            return ((prg_bank_count - 1) * 0x4000) + (cpu_addr & 0x3FFF);
    }
}

uint32_t Mapper001::MapChrAddress(const uint16_t ppu_addr) const {
    // CHR ROM banking logic

    // 8KB mode
    const uint8_t chr_mode = (control_ >> 4) & 0x01;
    if (chr_mode == 0) {
        return (chr_bank_0_ & 0x1E) * 0x1000 + (ppu_addr & 0x1FFF);
    }
    // 4KB mode
    if (ppu_addr < 0x1000)
        return (chr_bank_0_ * 0x1000) + (ppu_addr & 0x0FFF);

    return (chr_bank_1_ * 0x1000) + (ppu_addr & 0x0FFF);
}

uint8_t Mapper001::CpuRead(const uint16_t address) const {
    if (prg_rom_ && address >= 0x8000) {
        const uint32_t mapped = MapPrgAddress(address);
        if (mapped < prg_rom_->size())
            return (*prg_rom_)[mapped];
    }
    if (prg_ram_ && address >= 0x6000 && address < 0x8000) {
        const uint32_t ram_addr = (address - 0x6000) % prg_ram_->size();
        return (*prg_ram_)[ram_addr];
    }
    return 0;
}

void Mapper001::CpuWrite(const uint16_t address, const uint8_t data) {
    if (address >= 0x8000) {
        WriteRegister(address, data);
    }
    else if (prg_ram_ && address >= 0x6000 && address < 0x8000) {
        const uint32_t ram_addr = (address - 0x6000) % prg_ram_->size();
        (*prg_ram_)[ram_addr] = data;
    }
}

uint8_t Mapper001::PpuRead(const uint16_t address) const {
    const uint32_t mapped = MapChrAddress(address);
    if (chr_rom_ && mapped < chr_rom_->size())
        return (*chr_rom_)[mapped];
    if (chr_ram_ && mapped < chr_ram_->size())
        return (*chr_ram_)[mapped];
    return 0;
}

void Mapper001::PpuWrite(const uint16_t address, const uint8_t data) {
    const uint32_t mapped = MapChrAddress(address);
    if (chr_ram_ && mapped < chr_ram_->size())
        (*chr_ram_)[mapped] = data;
}
