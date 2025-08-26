#include "cartridge.h"

#include <iostream>
#include <filesystem>
#include <fstream>

#include "mappers/mapper_000.h"
#include "mappers/mapper_001.h"
#include "mappers/mapper_003.h"

Cartridge::Cartridge(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);

    if (!file) {
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        return;
    }

    // Parse the header
    if (!ParseHeader(file)) {
        std::cerr << "Invalid NES ROM header" << std::endl;
        return;
    }
    switch (mapper_id_) {
    case 0: mapper_ = std::make_shared<Mapper000>(header_.prg_rom_chunks_, header_.chr_rom_chunks_);
        break;
    case 1: mapper_ = std::make_shared<Mapper001>(header_.prg_rom_chunks_);
        break;
    case 3: mapper_ = std::make_shared<Mapper003>(header_.prg_rom_chunks_, header_.chr_rom_chunks_);
        break;
    default: ;
    }

    // Read PRG ROM with mirroring
    prg_rom_.resize(header_.prg_rom_chunks_ * 16 * 1024);
    file.read(reinterpret_cast<char*>(prg_rom_.data()), static_cast<std::streamsize>(prg_rom_.size()));

    // Read CHR ROM
    chr_rom_.resize(header_.chr_rom_chunks_ * 8 * 1024);
    file.read(reinterpret_cast<char*>(chr_rom_.data()), static_cast<std::streamsize>(chr_rom_.size()));

    size_t prg_ram_bytes = (header_.prg_ram_size_ == 0 ? 1 : header_.prg_ram_size_) * 8 * 1024;
    prg_ram_.resize(prg_ram_bytes);
    chr_ram_.resize(chr_rom_.empty() ? 8 * 1024 : 0); // 8KB default CHR RAM if no CHR ROM

    // Attach memory to mapper
    if (mapper_) {
        mapper_->prg_rom_ = &prg_rom_;
        mapper_->prg_ram_ = &prg_ram_;
        mapper_->chr_rom_ = &chr_rom_;
        mapper_->chr_ram_ = &chr_ram_;
    }

    nmi_vector_ = prg_rom_[prg_rom_.size() - 6] | (prg_rom_[prg_rom_.size() - 5] << 8);
    reset_vector_ = prg_rom_[prg_rom_.size() - 4] | (prg_rom_[prg_rom_.size() - 3] << 8);
    irq_vector_ = prg_rom_[prg_rom_.size() - 2] | (prg_rom_[prg_rom_.size() - 1] << 8);


    std::cout << "Loaded ROM: " << filename << std::endl;
    std::cout << "  Mapper: " << static_cast<int>(mapper_id_) << std::endl;
    std::cout << "  PRG ROM: " << static_cast<int>(header_.prg_rom_chunks_) << " x 16KB" << std::endl;
    std::cout << "  CHR ROM: " << static_cast<int>(header_.chr_rom_chunks_) << " x 8KB" << std::endl;

    loaded_ = true;
}

Cartridge::Cartridge() {
    // Initialize empty cartridge for unit testing
    loaded_ = true;
    mapper_id_ = 0;
    mirroring_ = MirroringType::kHorizontal;

    // Initialize empty vectors
    chr_rom_.resize(8 * 1024, 0); // 8KB CHR ROM
    prg_ram_.resize(8 * 1024, 0); // 8KB PRG RAM
    chr_ram_.resize(8 * 1024, 0); // 8KB CHR RAM

    mapper_ = std::make_shared<Mapper000>(1, 1);
    mapper_->prg_rom_ = &prg_rom_;
    mapper_->prg_ram_ = &prg_ram_;
    mapper_->chr_rom_ = &chr_rom_;
    mapper_->chr_ram_ = &chr_ram_;
}

bool Cartridge::ParseHeader(std::ifstream& file) {
    file.read(reinterpret_cast<char*>(&header_), sizeof(NesHeader));

    if (file.fail()) {
        return false;
    }

    // Check for NES header magic number
    if (header_.magic_[0] != 'N' || header_.magic_[1] != 'E' ||
        header_.magic_[2] != 'S' || header_.magic_[3] != 0x1A) {
        return false;
    }

    // Skip trainer if present
    if (header_.flags6_ & 0x04) {
        file.seekg(512, std::ios::cur);
    }

    // Set mapper ID
    mapper_id_ = (header_.flags7_ & 0xF0) | (header_.flags6_ >> 4);


    // Set mirroring type
    if (header_.flags6_ & 0x08) {
        mirroring_ = MirroringType::kFourScreen;
    }
    else if (header_.flags6_ & 0x01) {
        mirroring_ = MirroringType::kVertical;
    }
    else {
        mirroring_ = MirroringType::kHorizontal;
    }

    return true;
}

uint8_t Cartridge::CpuRead(const uint16_t address) const {
    if (!loaded_ || !mapper_) return 0;
    return mapper_->CpuRead(address);
}

void Cartridge::CpuWrite(const uint16_t address, const uint8_t data) const {
    if (!loaded_ || !mapper_) return;
    mapper_->CpuWrite(address, data);
}

uint8_t Cartridge::PpuRead(const uint16_t address) const {
    if (!loaded_ || !mapper_) return 0;
    return mapper_->PpuRead(address);
}

void Cartridge::PpuWrite(const uint16_t address, const uint8_t data) const {
    if (!loaded_ || !mapper_) return;
    mapper_->PpuWrite(address, data);
}
