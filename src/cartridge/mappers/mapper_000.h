#pragma once
#include "mapper_base.h"

class Mapper000 final : public MapperBase {
public:
    Mapper000(const uint8_t prg_chunks, const uint8_t chr_chunks): prg_chunks_(prg_chunks), chr_chunks_(chr_chunks) {
    }

    ~Mapper000() override = default;

    [[nodiscard]] uint8_t CpuRead(uint16_t address) const override;
    void CpuWrite(uint16_t address, uint8_t data) override;
    [[nodiscard]] uint8_t PpuRead(uint16_t address) const override;
    void PpuWrite(uint16_t address, uint8_t data) override;

private:
    uint8_t prg_chunks_;
    uint8_t chr_chunks_;

    [[nodiscard]] uint32_t MapPrgAddress(uint16_t cpu_addr) const override;
    [[nodiscard]] uint32_t MapChrAddress(uint16_t ppu_addr) const override;
};
