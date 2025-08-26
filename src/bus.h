#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include "cartridge/cartridge.h"

class CPU;
class PPU;

class Bus final {
public:
	explicit Bus(CPU* cpu, PPU* ppu);

	~Bus();

	// Memory operations
	[[nodiscard]] uint8_t Read(uint16_t address);
	void Write(uint16_t address, uint8_t value);


	bool LoadCartridge(const std::string& filename);

	// For unit testing
	bool InitEmptyCartridge();

	void Step();

	void DoDMA(uint8_t page);

	CPU* cpu_;
	PPU* ppu_;
	std::shared_ptr<Cartridge> cartridge_;
	uint32_t total_cycles_ = 0;
	std::array<uint8_t, 2 * 1024> ram_{}; // 2Kb of RAM (8 readable with mirroring)
	uint8_t curr_controller_state[2] = { 0, 0 };

	// Disassembly cache: address -> disassembly string
	std::map<uint16_t, std::string> disassembly_;
	// Generate disassembly for the entire PRG ROM
	void GenerateDisassembly();

	bool dma_active_ = false;

private:
	uint8_t controller_shift_reg[2];
	uint8_t dma_data_ = 0x00;
	uint8_t dma_addr_ = 0x00;  // DMA address index (0x00-0xFF inside a page)
	uint8_t dma_page_ = 0x00; // Current DMA page (0x00-0x7F)
	bool dma_dummy_ = true; // Dummy variable to synchronize DMA operations
};
