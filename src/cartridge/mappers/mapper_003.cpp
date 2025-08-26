#include "mapper_003.h"

Mapper003::Mapper003(const uint8_t prg_chunks, const uint8_t chr_chunks)
    : prg_chunks_(prg_chunks), chr_chunks_(chr_chunks) {
}

uint32_t Mapper003::MapPrgAddress(const uint16_t cpu_addr) const {
    // CNROM has fixed PRG ROM mapping
    if (cpu_addr >= 0x8000) {
        if (prg_chunks_ == 1)
            return cpu_addr & 0x3FFF; // Mirror 16KB PRG ROM
        else
            return cpu_addr & 0x7FFF; // 32KB PRG ROM
    }
    return 0xFFFFFFFF; // Invalid address
}

uint32_t Mapper003::MapChrAddress(const uint16_t ppu_addr) const {
    // Map PPU addresses ($0000-$1FFF) to selected CHR ROM bank
    if (ppu_addr <= 0x1FFF) {
        return (chr_bank_ * 0x2000) + (ppu_addr & 0x1FFF);
    }
    return 0xFFFFFFFF; // Invalid address
}

uint8_t Mapper003::CpuRead(const uint16_t address) const {
    // PRG RAM: $6000-$7FFF
    if (!prg_ram_->empty() && address >= 0x6000 && address <= 0x7FFF) {
        const uint32_t ram_addr = (address - 0x6000) % prg_ram_->size();
        return (*prg_ram_)[ram_addr];
    }
    // PRG ROM: $8000-$FFFF
    const uint32_t mapped = MapPrgAddress(address);
    if (prg_rom_ && mapped != 0xFFFFFFFF && mapped < prg_rom_->size())
        return (*prg_rom_)[mapped];
    return 0;
}

void Mapper003::CpuWrite(const uint16_t address, const uint8_t data) {
    // PRG RAM writes
    if (!prg_ram_->empty() && address >= 0x6000 && address <= 0x7FFF) {
        const uint32_t ram_addr = (address - 0x6000) % prg_ram_->size();
        (*prg_ram_)[ram_addr] = data;
    }
    // CHR bank select ($8000-$FFFF)
    else if (address >= 0x8000) {
        chr_bank_ = data & 0x03; // Only uses 2 bits for bank selection
    }
}

uint8_t Mapper003::PpuRead(const uint16_t address) const {
    const uint32_t mapped = MapChrAddress(address);
    if (chr_rom_ && mapped != 0xFFFFFFFF && mapped < chr_rom_->size())
        return (*chr_rom_)[mapped];
    if (chr_ram_ && mapped != 0xFFFFFFFF && mapped < chr_ram_->size())
        return (*chr_ram_)[mapped];
    return 0;
}

void Mapper003::PpuWrite(const uint16_t address, const uint8_t data) {
    const uint32_t mapped = MapChrAddress(address);
    // Only CHR RAM is writable
    if (chr_ram_ && mapped != 0xFFFFFFFF && mapped < chr_ram_->size())
        (*chr_ram_)[mapped] = data;
}
