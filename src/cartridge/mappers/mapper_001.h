#pragma once
#include "mapper_base.h"

// NES Mapper 1 (MMC1) implementation
class Mapper001 final : public MapperBase {
public:
    explicit Mapper001(uint8_t prg_chunks);
    ~Mapper001() override = default;

    [[nodiscard]] uint8_t CpuRead(uint16_t address) const override;
    void CpuWrite(uint16_t address, uint8_t data) override;
    [[nodiscard]] uint8_t PpuRead(uint16_t address) const override;
    void PpuWrite(uint16_t address, uint8_t data) override;

private:
    // Internal registers
    uint8_t shift_register_ = 0x10;
    uint8_t write_count_ = 0;
    uint8_t control_ = 0x0C;
    uint8_t chr_bank_0_ = 0;
    uint8_t chr_bank_1_ = 0;
    uint8_t prg_bank_ = 0;
    uint8_t prg_chunks_;

    // Helper functions
    void WriteRegister(uint16_t address, uint8_t data);
    [[nodiscard]] uint32_t MapPrgAddress(uint16_t cpu_addr) const override;
    [[nodiscard]] uint32_t MapChrAddress(uint16_t ppu_addr) const override;
};

