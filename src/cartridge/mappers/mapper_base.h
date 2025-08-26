#pragma once
#include <cstdint>
#include <vector>

class MapperBase {
public:
    MapperBase() = default;
    virtual ~MapperBase() = default;

    // Memory region pointers (set directly by Cartridge)
    std::vector<uint8_t>* prg_rom_ = nullptr;
    std::vector<uint8_t>* prg_ram_ = nullptr;
    std::vector<uint8_t>* chr_rom_ = nullptr;
    std::vector<uint8_t>* chr_ram_ = nullptr;

    // Map CPU address to PRG ROM offset
    [[nodiscard]] virtual uint32_t MapPrgAddress(uint16_t cpu_addr) const = 0;
    // Map PPU address to CHR ROM/RAM offset
    [[nodiscard]] virtual uint32_t MapChrAddress(uint16_t ppu_addr) const = 0;

    // Bus interface
    [[nodiscard]] virtual uint8_t CpuRead(uint16_t address) const = 0;
    virtual void CpuWrite(uint16_t address, uint8_t data) = 0;
    [[nodiscard]] virtual uint8_t PpuRead(uint16_t address) const = 0;
    virtual void PpuWrite(uint16_t address, uint8_t data) = 0;
};
