#include "mapper_000.h"

// Map CPU address ($8000-$FFFF) to PRG ROM offset
uint32_t Mapper000::MapPrgAddress(const uint16_t cpu_addr) const {
    if (cpu_addr >= 0x8000) {
        if (prg_chunks_ == 1)
            return cpu_addr & 0x3FFF; // Mirror 16KB
        else
            return cpu_addr & 0x7FFF; // 32KB
    }
    return 0xFFFFFFFF; // Invalid
}

// Map PPU address ($0000-$1FFF) to CHR ROM/RAM offset
uint32_t Mapper000::MapChrAddress(const uint16_t ppu_addr) const {
    // Only map addresses in $0000-$1FFF
    if (ppu_addr <= 0x1FFF)
        return ppu_addr;
    return 0xFFFFFFFF; // Invalid
}

uint8_t Mapper000::CpuRead(const uint16_t address) const {
    // PRG RAM: $6000-$7FFF (8KB, mirrored if less)
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

void Mapper000::CpuWrite(const uint16_t address, const uint8_t data) {
    // PRG RAM: $6000-$7FFF (8KB, mirrored if less)
    if (!prg_ram_->empty() && address >= 0x6000 && address <= 0x7FFF) {
        const uint32_t ram_addr = (address - 0x6000) % prg_ram_->size();
        (*prg_ram_)[ram_addr] = data;
    }
    // Writes to PRG ROM are ignored
}

uint8_t Mapper000::PpuRead(const uint16_t address) const {
    const uint32_t mapped = MapChrAddress(address);
    // NROM can read from CHR ROM or CHR RAM. Only read from CHR RAM if ROM doesn't exist.
    if (chr_rom_ && mapped != 0xFFFFFFFF && mapped < chr_rom_->size())
        return (*chr_rom_)[mapped];

    if (chr_ram_ && mapped != 0xFFFFFFFF && mapped < chr_ram_->size())
        return (*chr_ram_)[mapped];
    return 0;
}

void Mapper000::PpuWrite(const uint16_t address, const uint8_t data) {
    const uint32_t mapped = MapChrAddress(address);
    // Only CHR RAM is writable
    if (chr_rom_ == nullptr && chr_ram_ && mapped != 0xFFFFFFFF && mapped < chr_ram_->size())
        (*chr_ram_)[mapped] = data;
}
