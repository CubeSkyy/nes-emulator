#pragma once
#include "mapper_base.h"

// NES Mapper 3 (CNROM) implementation
class Mapper003 final : public MapperBase {
public:
    explicit Mapper003(uint8_t prg_chunks, uint8_t chr_chunks);
    ~Mapper003() override = default;

    [[nodiscard]] uint8_t CpuRead(uint16_t address) const override;
    void CpuWrite(uint16_t address, uint8_t data) override;
    [[nodiscard]] uint8_t PpuRead(uint16_t address) const override;
    void PpuWrite(uint16_t address, uint8_t data) override;

private:
    uint8_t prg_chunks_;
    uint8_t chr_chunks_;
    uint8_t chr_bank_ = 0;

    [[nodiscard]] uint32_t MapPrgAddress(uint16_t cpu_addr) const override;
    [[nodiscard]] uint32_t MapChrAddress(uint16_t ppu_addr) const override;
};
