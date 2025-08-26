#include "ppu.h"

#include <cstring>
#include <vector>

#include "cartridge/cartridge.h"

uint8_t PPU::PpuRead(uint16_t address) const {
	if (address >= 0x2000 && address < 0x3F00) {
		address &= 0x0FFF;
		int table = 0;
		if (cartridge_->mirroring_ == Cartridge::MirroringType::kVertical) {
			table = (address & 0x400) ? 1 : 0;
		}
		else if (cartridge_->mirroring_ == Cartridge::MirroringType::kHorizontal) {
			table = (address & 0x800) ? 1 : 0;
		}
		return name_table_[table][address & 0x3FF];
	}
	else if (address >= 0x3F00 && address <= 0x3FFF) {
		address &= 0x1F; // Mirror down to 0-31 range

		// Handle mirroring of $3F10/$3F14/$3F18/$3F1C to $3F00/$3F04/$3F08/$3F0C
		if ((address & 0x13) == 0x10) {
			address &= ~0x10;
		}

		uint8_t color = palette_buffer_[address];
		return color & (mask_.grayscale_ ? 0x30 : 0x3F);
	}

	// Delegate to cartridge
	return cartridge_->PpuRead(address);
}

void PPU::PpuWrite(uint16_t address, const uint8_t value) {
	if (address >= 0x2000 && address < 0x3F00) {
		address &= 0x0FFF;
		int table = 0;
		if (cartridge_->mirroring_ == Cartridge::MirroringType::kVertical) {
			table = (address & 0x400) ? 1 : 0;
		}
		else if (cartridge_->mirroring_ == Cartridge::MirroringType::kHorizontal) {
			table = (address & 0x800) ? 1 : 0;
		}
		name_table_[table][address & 0x3FF] = value;
		return;
	}
	else if (address >= 0x3F00 && address <= 0x3FFF) {
		address &= 0x1F; // Mirror down to 0-31 range

		// Handle mirroring of $3F10/$3F14/$3F18/$3F1C to $3F00/$3F04/$3F08/$3F0C
		if ((address & 0x13) == 0x10) {
			address &= ~0x10;
		}

		// Handle additional palette mirroring
		if (address == 0x04) palette_buffer_[0x14] = value;
		else if (address == 0x08) palette_buffer_[0x18] = value;
		else if (address == 0x0C) palette_buffer_[0x1C] = value;

		palette_buffer_[address] = value;
	}
	else {
		// Delegate to cartridge
		cartridge_->PpuWrite(address, value);
	}
}

uint8_t PPU::CpuRead(uint16_t address) {
	uint8_t data = 0x00;

	if (address >= 0x0000 && address <= 0x0007) {
		switch (address) {
		case 0x0002: // PPU status register
			data = (status_.value_ & 0xE0) | (data_buffer_ & 0x1F);
			status_.vertical_blank_ = 0;
			write_toggle_ = false;
			break;
		case 0x0004: // OAM data register (OAMDATA)
			data = oam_.bytes[oam_address_];
			break;

		case 0x0007: // PPU data register
			if (vram_address_.value_ < 0x3F00) {
				data = data_buffer_;
				data_buffer_ = PpuRead(vram_address_.value_);
			}
			else {
				// Palette reads are not buffered
				data = PpuRead(vram_address_.value_);
				data_buffer_ = PpuRead(vram_address_.value_ & 0x2FFF); // mimic buffer update
			}

			vram_address_.value_ += (ctrl_.increment_mode_ ? 32 : 1);
			break;
		default:
			break;
		}
	}

	return data;
}

void PPU::CpuWrite(uint16_t address, uint8_t value) {
	switch (address) {
	case 0x0000: // PPU control register
		ctrl_.value_ = value;
		tram_address_.value_ = (tram_address_.value_ & 0xF3FF) | ((value & 0x03) << 10);
		break;
	case 0x0001: // PPU mask register
		mask_.value_ = value;
		break;
	case 0x0003: // OAM address register (OAMADDR)
		oam_address_ = value;
		break;
	case 0x0004: // OAM data register (OAMDATA)
		oam_.bytes[oam_address_] = value;
		break;
	case 0x0005: // PPU scroll register
		if (!write_toggle_) {
			fine_x_ = value & 0x07; // Fine X scroll (0-7)
			tram_address_.coarse_x_ = value >> 3; // Coarse X scroll (0-31)
			write_toggle_ = !write_toggle_;
		}
		else {
			tram_address_.fine_y_ = value & 0x07; // Fine Y scroll (0-7)
			tram_address_.coarse_y_ = value >> 3; // Coarse Y scroll (0-31)
			write_toggle_ = !write_toggle_;
		}
		break;
	case 0x0006: // PPU address register
		if (!write_toggle_) {
			// First write: high byte (bits 8-14)
			//tram_address_.value_ = (tram_address_.value_ & 0x00FF) | ((value & 0x3F) << 8);
			tram_address_.value_ = (uint16_t)((value & 0x3F) << 8) | (tram_address_.value_ & 0x00FF);
		}
		else {
			// Second write: low byte (bits 0-7)
			tram_address_.value_ = (tram_address_.value_ & 0xFF00) | value;
			vram_address_ = tram_address_;
		}
		tram_address_.value_ &= 0x3FFF;
		write_toggle_ = !write_toggle_;
		break;
	case 0x0007: // PPU data register
		PpuWrite(vram_address_.value_, value);
		vram_address_.value_ += (ctrl_.increment_mode_ ? 32 : 1);
		break;
	default:
		break;
	}
}


void PPU::Step() {
	// Following NESDev timings

	// Functions for readability
	auto ReverseByte = [](uint8_t b) -> uint8_t {
		// Mirrors Byte horizontally
		// Input: 0b00000101 becomes
		//        0b10100000
		b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
		b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
		b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
		return b;
	};

	auto SetOffsetId = [&]() {
		// Last 3 bytes of vram_address_ index the 4 nametables.
		// Thus they serve as an offset for 0x2000 (Nametable address space start)
		bg_tile_id_ = PpuRead(0x2000 | (vram_address_.value_ & 0x0FFF));
	};

	auto SetAttByte = [&]() {
		bg_attribute_ = PpuRead(0x23C0
			| (vram_address_.nametable_y_ << 11)
			| (vram_address_.nametable_x_ << 10)
			| ((vram_address_.coarse_y_ >> 2) << 3)
			| (vram_address_.coarse_x_ >> 2));
		if (vram_address_.coarse_y_ & 0x02) bg_attribute_ >>= 4;
		if (vram_address_.coarse_x_ & 0x02) bg_attribute_ >>= 2;
		bg_attribute_ &= 0x03;
	};

	auto SetBgLsb = [&]() {
		// Address: 1. 0x0000 or 0x1000 based on control register
		//          2. Tile offset (tile ID × 16 bytes per tile)
		//          3. Fine Y (0-7), specific pixel
		bg_lsb_ = PpuRead((ctrl_.background_pattern_table_ << 12)
			+ ((uint16_t)bg_tile_id_ << 4)
			+ (vram_address_.fine_y_ + 0));
	};

	auto SetBgMsb = [&]() {
		// Same as LSB + 8 bytes
		bg_msb_ = PpuRead((ctrl_.background_pattern_table_ << 12)
			+ ((uint16_t)bg_tile_id_ << 4)
			+ (vram_address_.fine_y_ + 8));
	};

	auto IncrementX = [&]() {
		if (mask_.show_background_ || mask_.show_sprites_) {
			if (vram_address_.coarse_x_ == 31) {
				// Name table is 32x30, so we wrap to the start of next nametable
				vram_address_.coarse_x_ = 0;
				vram_address_.nametable_x_ = ~vram_address_.nametable_x_;
			}
			else {
				// Otherwise, increment X
				vram_address_.coarse_x_++;
			}
		}
	};

	auto IncrementY = [&]() {
		if (mask_.show_background_ || mask_.show_sprites_) {
			// If fine_y < 7, just increment it
			if (vram_address_.fine_y_ < 7) {
				vram_address_.fine_y_++;
			}
			else {
				// If we're at fine_y = 7, we need to increment coarse Y
				// and reset fine_y to 0
				vram_address_.fine_y_ = 0;

				if (vram_address_.coarse_y_ == 29) {
					// We've hit the bottom of the nametable
					vram_address_.coarse_y_ = 0;
					vram_address_.nametable_y_ = ~vram_address_.nametable_y_;
				}
				else if (vram_address_.coarse_y_ == 31) {
					// We've hit the bottom of the attribute table
					vram_address_.coarse_y_ = 0;
				}
				else {
					vram_address_.coarse_y_++;
				}
			}
		}
	};

	auto UpdateVramX = [&]() {
		// Cycle 257: Copy horizontal scrolling data from temporary VRAM address to the actual VRAM address
		// This happens every scanline and is how the PPU supports horizontal scrolling
		// By copying at scanline end, we ensure smooth scrolling even if game updates PPUSCROLL mid-frame
		// and also allows for parallax by settings different scroll for different lines
		if (mask_.show_background_ || mask_.show_sprites_) {
			vram_address_.nametable_x_ = tram_address_.nametable_x_;
			vram_address_.coarse_x_ = tram_address_.coarse_x_;
		}
	};

	auto UpdateVramY = [&]() {
		// During pre-render scanline (-1), copy vertical scrolling data from temporary VRAM address
		// This happens between cycles 280 and 304, once per frame
		// This is how the PPU supports vertical scrolling, updating just before the new frame starts
		// Multiple cycles are used for this operation as PPU needs time to synchronize counters
		if (mask_.show_background_ || mask_.show_sprites_) {
			// Copy all vertical scroll components from temp to vram address
			vram_address_.fine_y_ = tram_address_.fine_y_;
			vram_address_.nametable_y_ = tram_address_.nametable_y_;
			vram_address_.coarse_y_ = tram_address_.coarse_y_;
		}
	};

	auto LoadBackgroundShiftRegisters = [&]() {
		// Pattern data - shift left by 8 and load new tile data
		bg_lsb_shift_reg = (bg_lsb_shift_reg & 0xFF00) | bg_lsb_;
		bg_msb_shift_reg = (bg_msb_shift_reg & 0xFF00) | bg_msb_;

		// Attribute data - shift left by 8 and expand single bits into full bytes (0x00 or 0xFF)
		bg_att_lsb_shift_reg = (bg_att_lsb_shift_reg & 0xFF00) | ((bg_attribute_ & 0b01) ? 0xFF : 0x00);
		bg_att_msb_shift_reg = (bg_att_msb_shift_reg & 0xFF00) | ((bg_attribute_ & 0b10) ? 0xFF : 0x00);
	};

	auto ShiftRenderingRegisters = [&]() {
		if (mask_.show_background_) {
			bg_lsb_shift_reg <<= 1;
			bg_msb_shift_reg <<= 1;
			bg_att_lsb_shift_reg <<= 1;
			bg_att_msb_shift_reg <<= 1;
		}

		if (mask_.show_sprites_ && cycle_ >= 1 && cycle_ <= 257) {
			// Shift sprite shift registers only during visible cycles
			for (int i = 0; i < curr_scanline_sprite_count_; i++) {
				if (curr_scanline_sprites_[i].x_ > 0) {
					curr_scanline_sprites_[i].x_--;
				}
				else {
					sprite_lsb_shift_reg[i] <<= 1;
					sprite_msb_shift_reg[i] <<= 1;
				}
			}
		}
	};

	// Visible scanlines/cycles
	if (scanline_ >= -1 && scanline_ < 240) {
		// Off frame skip
		if (scanline_ == 0 && cycle_ == 0 && is_odd_frame_ && (mask_.show_background_ || mask_.show_sprites_)) {
			cycle_ = 1;
		}

		// Start of new frame
		if (scanline_ == -1 && cycle_ == 1) {
			// Clear vblank flag and reset variables
			status_.vertical_blank_ = 0;
			status_.sprite_overflow_ = 0;
			status_.sprite_0_hit_ = 0;

			for (int i = 0; i < 8; i++) {
				sprite_lsb_shift_reg[i] = 0;
				sprite_msb_shift_reg[i] = 0;
			}
		}

		// Loading pixel data for visible pixels
		if ((cycle_ >= 2 && cycle_ <= 257) || (cycle_ >= 321 && cycle_ <= 337)) {
			ShiftRenderingRegisters();

			// Every 8 cycles, data for 8 pixels is loaded
			switch ((cycle_ - 1) % 8) {
			case 0: // NT fetch
				LoadBackgroundShiftRegisters();
				SetOffsetId();
				break;
			case 2: // AT fetch
				SetAttByte();
				break;
			case 4: // Low BG tile byte
				SetBgLsb();
				break;
			case 6: // High BG tile byte
				SetBgMsb();
				break;
			case 7: // Increment scroll counters
				IncrementX();
				break;
			}
		}

		// Last visible cycle, increase Y
		if (cycle_ == 256) {
			IncrementY();
		}

		// Horizontal scrolling
		if (cycle_ == 257) {
			LoadBackgroundShiftRegisters();
			UpdateVramX();
		}

		// Dummy read
		if (cycle_ == 338 || cycle_ == 340) {
			bg_tile_id_ = PpuRead(0x2000 | (vram_address_.value_ & 0x0FFF));
		}

		// Vertical scrolling - only during pre-render scanline
		if (scanline_ == -1 && cycle_ >= 280 && cycle_ <= 304) {
			UpdateVramY();
		}

		// Foreground rendering

		// Sprite Evaluation Phase: Read OAM for possible sprites on this scanline
		if (cycle_ == 257 && scanline_ >= 0) {
			// Initialize sprites as off-screen (Y=0xFF makes sprite invisible)
			std::memset(curr_scanline_sprites_, 0xFF, 8 * sizeof(Sprite));
			curr_scanline_sprite_count_ = 0; // Clear current scanline sprite count

			for (uint8_t i = 0; i < 8; i++) {
				sprite_lsb_shift_reg[i] = 0;
				sprite_msb_shift_reg[i] = 0;
			}

			uint8_t oamCurrIndex = 0;

			spriteZeroHitPossible_ = false;

			while (oamCurrIndex < 64 && curr_scanline_sprite_count_ < 9) {
				int16_t diff = ((int16_t)scanline_ - (int16_t)oam_.sprites[oamCurrIndex].y_);
				// If the difference is positive then the scanline is at least at the
				// same height as the sprite, so check if it resides in the sprite vertically
				// depending on the current "sprite height mode"
				// FLAGGED

				if (diff >= 0 && diff < (ctrl_.sprite_size_ ? 16 : 8) && curr_scanline_sprite_count_ < 8) {
					// Sprite is visible, so copy the attribute entry over to our
					// scanline sprite cache. Ive added < 8 here to guard the array
					// being written to.
					if (curr_scanline_sprite_count_ < 8) {
						// Is this sprite sprite zero?
						if (oamCurrIndex == 0) {
							// It is, so its possible it may trigger a
							// sprite zero hit when drawn
							spriteZeroHitPossible_ = true;
						}

						curr_scanline_sprites_[curr_scanline_sprite_count_] = oam_.sprites[oamCurrIndex];
					}
					curr_scanline_sprite_count_++;
				}
				oamCurrIndex++;
			}

			status_.sprite_overflow_ = curr_scanline_sprite_count_ > 8;
		}


		// Sprite Rendering Phase: Load sprite pattern data into shift registers
		if (cycle_ == 340) {
			// Now we're at the very end of the scanline, I'm going to prepare the 
			// sprite shifters with the 8 or less selected sprites.

			for (uint8_t i = 0; i < curr_scanline_sprite_count_; i++) {
				// We need to extract the 8-bit row patterns of the sprite with the
				// correct vertical offset. The "Sprite Mode" also affects this as
				// the sprites may be 8 or 16 rows high. Additionally, the sprite
				// can be flipped both vertically and horizontally. So there's a lot
				// going on here :P

				uint8_t sprite_pattern_bits_lo, sprite_pattern_bits_hi;
				uint16_t sprite_pattern_addr_lo, sprite_pattern_addr_hi;

				// Determine the memory addresses that contain the byte of pattern data. We
				// only need the lo pattern address, because the hi pattern address is always
				// offset by 8 from the lo address.
				if (!ctrl_.sprite_size_) {
					// 8x8 Sprite Mode - The control register determines the pattern table
					if (!(curr_scanline_sprites_[i].flip_v_)) {
						// Sprite is NOT flipped vertically, i.e. normal    
						sprite_pattern_addr_lo =
							(ctrl_.sprite_pattern_table_ << 12) // Which Pattern Table? 0KB or 4KB offset
							| (curr_scanline_sprites_[i].tile_id_ << 4) // Which Cell? Tile ID * 16 (16 bytes per tile)
							| (scanline_ - curr_scanline_sprites_[i].y_); // Which Row in cell? (0->7)
					}
					else {
						// Sprite is flipped vertically, i.e. upside down
						sprite_pattern_addr_lo =
							(ctrl_.sprite_pattern_table_ << 12) // Which Pattern Table? 0KB or 4KB offset
							| (curr_scanline_sprites_[i].tile_id_ << 4) // Which Cell? Tile ID * 16 (16 bytes per tile)
							| (7 - (scanline_ - curr_scanline_sprites_[i].y_)); // Which Row in cell? (7->0)
					}
				}
				else {
					// 8x16 Sprite Mode - The sprite attribute determines the pattern table
					if (!(curr_scanline_sprites_[i].flip_v_)) {
						// Sprite is NOT flipped vertically, i.e. normal
						if (scanline_ - curr_scanline_sprites_[i].y_ < 8) {
							// Reading Top half Tile
							sprite_pattern_addr_lo =
								((curr_scanline_sprites_[i].tile_id_ & 0x01) << 12) // Which Pattern Table? 0KB or 4KB offset
								| ((curr_scanline_sprites_[i].tile_id_ & 0xFE) << 4) // Which Cell? Tile ID * 16 (16 bytes per tile)
								| ((scanline_ - curr_scanline_sprites_[i].y_) & 0x07); // Which Row in cell? (0->7)
						}
						else {
							// Reading Bottom Half Tile
							sprite_pattern_addr_lo =
								((curr_scanline_sprites_[i].tile_id_ & 0x01) << 12) // Which Pattern Table? 0KB or 4KB offset
								| (((curr_scanline_sprites_[i].tile_id_ & 0xFE) + 1) << 4)
								// Which Cell? Tile ID * 16 (16 bytes per tile)
								| ((scanline_ - curr_scanline_sprites_[i].y_) & 0x07); // Which Row in cell? (0->7)
						}
					}
					else {
						// Sprite is flipped vertically, i.e. upside down
						if (scanline_ - curr_scanline_sprites_[i].y_ < 8) {
							// Reading Top half Tile
							sprite_pattern_addr_lo =
								((curr_scanline_sprites_[i].tile_id_ & 0x01) << 12) // Which Pattern Table? 0KB or 4KB offset
								| (((curr_scanline_sprites_[i].tile_id_ & 0xFE) + 1) << 4)
								// Which Cell? Tile ID * 16 (16 bytes per tile)
								| (7 - (scanline_ - curr_scanline_sprites_[i].y_) & 0x07); // Which Row in cell? (0->7)
						}
						else {
							// Reading Bottom Half Tile
							sprite_pattern_addr_lo =
								((curr_scanline_sprites_[i].tile_id_ & 0x01) << 12) // Which Pattern Table? 0KB or 4KB offset
								| ((curr_scanline_sprites_[i].tile_id_ & 0xFE) << 4) // Which Cell? Tile ID * 16 (16 bytes per tile)
								| (7 - (scanline_ - curr_scanline_sprites_[i].y_) & 0x07); // Which Row in cell? (0->7)
						}
					}
				}

				// Phew... XD I'm absolutely certain you can use some fantastic bit 
				// manipulation to reduce all of that to a few one liners, but in this
				// form it's easy to see the processes required for the different
				// sizes and vertical orientations

				// Hi bit plane equivalent is always offset by 8 bytes from lo bit plane
				sprite_pattern_addr_hi = sprite_pattern_addr_lo + 8;

				// Now we have the address of the sprite patterns, we can read them
				sprite_pattern_bits_lo = PpuRead(sprite_pattern_addr_lo);
				sprite_pattern_bits_hi = PpuRead(sprite_pattern_addr_hi);

				// If the sprite is flipped horizontally, we need to flip the 
				// pattern bytes. 
				if (curr_scanline_sprites_[i].flip_h_) {
					// This little lambda function "flips" a byte
					// so 0b11100000 becomes 0b00000111. It's very
					// clever, and stolen completely from here:
					// https://stackoverflow.com/a/2602885
					auto flipbyte = [](uint8_t b) {
						b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
						b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
						b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
						return b;
					};

					// Flip Patterns Horizontally
					sprite_pattern_bits_lo = flipbyte(sprite_pattern_bits_lo);
					sprite_pattern_bits_hi = flipbyte(sprite_pattern_bits_hi);
				}

				// Finally! We can load the pattern into our sprite shift registers
				// ready for rendering on the next scanline
				sprite_lsb_shift_reg[i] = sprite_pattern_bits_lo;
				sprite_msb_shift_reg[i] = sprite_pattern_bits_hi;
			}
		}
	}

	// Start of Post-Render Stage
	if (scanline_ == 241 && cycle_ == 1) {
		status_.vertical_blank_ = 1;
		if (ctrl_.enable_nmi_) {
			was_nmi_triggered_ = true;
		}
	}

	// Background Rendering: Calculate BG tiles
	uint8_t bg_px_idx = 0x00;
	uint8_t bg_pal_idx = 0x00;
	if (mask_.show_background_) {
		// Calculate the current pixel index based on scrolling
		uint16_t curr_bit_index = (1 << 15) >> fine_x_;

		// Extract pattern (pixel) bits
		uint8_t p0 = (bg_lsb_shift_reg & curr_bit_index) ? 1 : 0;
		uint8_t p1 = (bg_msb_shift_reg & curr_bit_index) ? 1 : 0;
		bg_px_idx = (p1 << 1) | p0;

		// Extract palette bits
		uint8_t pal0 = (bg_att_lsb_shift_reg & curr_bit_index) ? 1 : 0;
		uint8_t pal1 = (bg_att_msb_shift_reg & curr_bit_index) ? 1 : 0;
		bg_pal_idx = (pal1 << 1) | pal0;
	}

	// Foreground Rendering: Calculate sprite tiles
	uint8_t fg_px_idx = 0x00;
	uint8_t fg_pal_idx = 0x00;
	uint8_t fg_prio = 0x00;
	if (mask_.show_sprites_) {
		spriteZeroRendered_ = false;

		// Check each sprite for rendering
		for (int i = 0; i < curr_scanline_sprite_count_; i++) {
			const auto& sprite = curr_scanline_sprites_[i];
			if (sprite.x_ == 0) {
				// Sprite decrement hit 0, time to draw it

				// Extract pattern (pixel) bits
				uint8_t p0 = (sprite_lsb_shift_reg[i] & 0x80) > 0;
				uint8_t p1 = (sprite_msb_shift_reg[i] & 0x80) > 0;
				fg_px_idx = (p1 << 1) | p0;

				if (fg_px_idx == 0) continue; // Transparent pixel, skip

				fg_pal_idx = sprite.palette_ + 4;
				fg_prio = sprite.priority_;

				if (fg_px_idx != 0) {
					if (i == 0) {
						spriteZeroRendered_ = true;
					}

					break;
				}
			}
		}
	}

	// Final stage, render the actual pixel
	if (scanline_ >= 0 && scanline_ < 240 && cycle_ >= 1 && cycle_ < 256) {
		if (fg_px_idx != 0) {
			// Foreground pixel is not transparent
			if (bg_px_idx == 0 || !fg_prio) {
				// Render sprite if:
				// 1. Background is transparent, OR
				// 2. Sprite has priority (priority_ = 0 means in front)
				framebuffer_[scanline_ * kWidth + (cycle_ - 1)] = ResolvePaletteRamColor(fg_pal_idx, fg_px_idx);
			}
			else {
				// Background is not transparent and won sprite priority
				framebuffer_[scanline_ * kWidth + (cycle_ - 1)] = ResolvePaletteRamColor(bg_pal_idx, bg_px_idx);
			}

			if (bg_px_idx != 0 && spriteZeroHitPossible_ && spriteZeroRendered_ && mask_.show_background_ && mask_.
				show_sprites_) {
				// Check if we're in the valid range for sprite 0 hit
				if (~(mask_.show_background_leftmost_8px_ | mask_.show_sprites_leftmost_8px_)) {
					// If neither sprite nor BG rendering is enabled for left 8 pixels,
					// sprite zero hit cannot occur in that region
					if (cycle_ >= 9 && cycle_ < 258) {
						status_.sprite_0_hit_ = 1;
					}
				}
				else {
					// Both sprite and BG rendering enabled for left pixels
					if (cycle_ >= 1 && cycle_ < 258) {
						status_.sprite_0_hit_ = 1;
					}
				}
			}
		}
		else if (bg_px_idx != 0) {
			// Background pixel is not transparent and Foreground is transparent
			framebuffer_[scanline_ * kWidth + (cycle_ - 1)] = ResolvePaletteRamColor(bg_pal_idx, bg_px_idx);
		}
		else {
			// Both pixels are transparent, use universal background color
			framebuffer_[scanline_ * kWidth + (cycle_ - 1)] = ResolvePaletteRamColor(0, 0);
		}
	}

	cycle_++;
	if (cycle_ >= 341) {
		cycle_ = 0;
		scanline_++;
		if (scanline_ >= 261) {
			scanline_ = -1;
			frame_complete_ = true;
			is_odd_frame_ = !is_odd_frame_;
		}
	}
}

std::vector<PPU::Pixel> PPU::GetPatternTableSprite(const int table_idx, const int palette_id) {
	std::vector<Pixel> sprite(128 * 128);
	const int base_addr = table_idx * 0x1000; // 4KB per pattern table
	for (int tile_y = 0; tile_y < 16; ++tile_y) {
		for (int tile_x = 0; tile_x < 16; ++tile_x) {
			const int tile_addr = base_addr + tile_y * 256 + tile_x * 16;
			for (int row = 0; row < 8; ++row) {
				uint8_t plane0 = PpuRead(tile_addr + row);
				uint8_t plane1 = PpuRead(tile_addr + row + 8);
				for (int col = 0; col < 8; ++col) {
					// Extract the bit for this pixel from each bitplane
					// Reading rightmost bit first since NES uses little-endian for pattern tiles
					const uint8_t bit0 = (plane0 >> (7 - col)) & 1;
					const uint8_t bit1 = (plane1 >> (7 - col)) & 1;
					// Combine the two bits to get the NES color index (0-3)
					uint8_t color_idx = (bit1 << 1) | bit0;

					// Look up the RGB value from the NES palette
					const uint16_t palette_addr = 0x3F00 + palette_id * 4 + color_idx;
					const uint8_t palette_color_index = PpuRead(palette_addr) & 0x3F;
					const Pixel px = GetPaletteColor(palette_color_index);
					const int x = tile_x * 8 + (7 - col); // Flip X position since we read bits from right
					const int y = tile_y * 8 + row;
					sprite[y * 128 + x] = px;
				}
			}
		}
	}
	return sprite;
}
