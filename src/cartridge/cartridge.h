#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "mappers/mapper_base.h"


class Cartridge {
public:
    // =====================
    // === Types & Consts ===
    // =====================
    struct NesHeader {
        uint8_t magic_[4]; // 'NES' followed by MS-DOS EOF (0x1A)
        uint8_t prg_rom_chunks_; // PRG ROM size in 16KB units
        uint8_t chr_rom_chunks_; // CHR ROM size in 8KB units
        uint8_t flags6_; // Mapper, mirroring, battery, trainer
        uint8_t flags7_; // Mapper, VS/Playchoice, NES 2.0
        uint8_t prg_ram_size_; // PRG-RAM size in 8KB units
        uint8_t flags9_; // TV system (rarely used)
        uint8_t flags10_; // TV system, PRG-RAM presence (rarely used)
        uint8_t padding_[5]; // Unused padding
    };

    enum class MirroringType {
        kHorizontal,
        kVertical,
        kFourScreen,
        kSingleScreenLower,
        kSingleScreenUpper
    };

    // =====================
    // === Public API ======
    // =====================
    uint16_t nmi_vector_{}, reset_vector_{}, irq_vector_{};

    // Constructor loads a ROM from the specified file path
    explicit Cartridge(const std::string& filename);
    explicit Cartridge();
    // Bus interface for CPU and PPU
    [[nodiscard]] uint8_t CpuRead(uint16_t address) const;
    void CpuWrite(uint16_t address, uint8_t data) const;
    [[nodiscard]] uint8_t PpuRead(uint16_t address) const;
    void PpuWrite(uint16_t address, uint8_t data) const;
    [[nodiscard]] bool isLoaded() const { return loaded_; }

    std::vector<uint8_t> prg_rom_;
    std::vector<uint8_t> prg_ram_;

    // Patern Memory, 2 banks of  16x16 tiles, 8x8 pixels each
    std::vector<uint8_t> chr_rom_;
    std::vector<uint8_t> chr_ram_; // Used if chr_rom is empty

    MirroringType mirroring_ = MirroringType::kHorizontal;
private:
    // =====================
    // === Internal State ==
    // =====================
    bool loaded_ = false;
    uint8_t mapper_id_ = 0;
    std::shared_ptr<MapperBase> mapper_;


    NesHeader header_{};


    // Parse the iNES header
    bool ParseHeader(std::ifstream& file);
};

#endif // CARTRIDGE_H
