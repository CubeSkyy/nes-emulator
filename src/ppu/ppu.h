#pragma once
#include <cstdint>
#include <memory>
#include <set>
#include <vector>


class Cartridge;

class PPU {
public:
    // =====================
    // === Types & Consts ===
    // =====================
    struct Pixel {
        uint8_t r_, g_, b_;

        bool operator<(const Pixel& other) const {
            if (r_ != other.r_) return r_ < other.r_;
            if (g_ != other.g_) return g_ < other.g_;
            return b_ < other.b_;
        }
    };

    union Sprite {
        struct {
            uint8_t y_; // Y position minus 1 (0-239)
            uint8_t tile_id_; // Tile number from pattern table
            struct {
                uint8_t palette_ : 2; // Palette (0-3) from attribute table
                uint8_t unused_ : 3; // Unused bits
                uint8_t flip_h_ : 1; // Flip sprite horizontally
                uint8_t flip_v_ : 1; // Flip sprite vertically
                uint8_t priority_ : 1; // 0: in front of background, 1: behind background
            };

            uint8_t x_; // X position (0-255)
        };

        uint8_t bytes[4]; // Raw byte access to sprite data
    };

    // OAM memory union for efficient access
    union OAMMemory {
        Sprite sprites[64]; // Structured sprite access
        uint8_t bytes[256]; // Raw byte access
    };

    static constexpr int kWidth = 256;
    static constexpr int kHeight = 240;

    static constexpr Pixel GetPaletteColor(const uint8_t idx) { return kPalette[idx]; }

    uint8_t GetPaletteRamColor(const uint8_t pal_idx, const uint8_t px_idx) const {
        return PpuRead(0x3F00 + pal_idx * 4 + px_idx) & 0x3F;
    }

    Pixel ResolvePaletteRamColor(const uint8_t pal_idx, const uint8_t px_idx) const {
        return GetPaletteColor(GetPaletteRamColor(pal_idx, px_idx));
    }

    // =====================
    // === Public API ======
    // =====================
    PPU() = default;
    ~PPU() = default;

    // Cartridge interface
    std::shared_ptr<Cartridge> cartridge_;
    bool was_nmi_triggered_;
    bool is_odd_frame_ = false;
    bool spriteZeroHitPossible_;
    bool spriteZeroRendered_;

    // PPU memory access
    // Called by the CPU via the Bus to access PPU registers ($2000-$2007)
    uint8_t CpuRead(uint16_t address);
    void CpuWrite(uint16_t address, uint8_t value);

    // Called internally by the PPU for direct VRAM/palette/OAM access
    uint8_t PpuRead(uint16_t address) const;
    void PpuWrite(uint16_t address, uint8_t value);

    // Framebuffer access
    const std::vector<Pixel>& GetFrameBuffer() const { return framebuffer_; }
    bool frame_complete_ = false;
    uint8_t palette_buffer_[32] = {};
    OAMMemory oam_; // OAM (Object Attribute Memory) for sprites, 64 entries (256 bytes total)
    Sprite curr_scanline_sprites_[8]; // Sprites visible on the current scanline (max 8)
    uint8_t curr_scanline_sprite_count_ = 0; // If sprites exceed 8, the 9th sprite flag is set

    // Emulation step
    void Step();

    // Debugging
    std::vector<Pixel> GetPatternTableSprite(int table_idx, int palette_id);

    // =====================
    // === NES Registers ===
    // =====================
    // Loopy register
    union LoopyRegister {
        struct {
            uint16_t coarse_x_ : 5; // Coarse X scroll inside name table (0-31)
            uint16_t coarse_y_ : 5; // Coarse Y scroll inside name table (0-29)
            uint16_t nametable_x_ : 1; // Nametable select (0-3)
            uint16_t nametable_y_ : 1; // Nametable select (0-3)
            uint16_t fine_y_ : 3; // Fine Y scroll (0-7)
            uint16_t unused_ : 1; // Unused bit, always 0
        };

        uint16_t value_ = 0x0000;
    };

    uint8_t fine_x_ = 0x00;

    LoopyRegister vram_address_;
    LoopyRegister tram_address_; // Temporary VRAM address for PPUADDR writes

    // Data for BG pixel rendering that will be rendered by the shift registers
    uint8_t bg_tile_id_ = 0x00;
    uint8_t bg_attribute_ = 0x00;
    uint8_t bg_lsb_ = 0x00;
    uint8_t bg_msb_ = 0x00;

    // Shift registers
    uint16_t bg_att_lsb_shift_reg = 0x0000;
    uint16_t bg_att_msb_shift_reg = 0x0000;
    uint16_t bg_lsb_shift_reg = 0x0000;
    uint16_t bg_msb_shift_reg = 0x0000;


    uint8_t sprite_lsb_shift_reg[8]; // Shift registers for 8 sprites
    uint8_t sprite_msb_shift_reg[8]; // Shift registers for 8 sprites

    //  PPU REGISTER STRUCTURES
    // PPUCTRL (the "control" or "controller" register) contains a mix of settings related to rendering
    // scroll position, vblank NMI, and dual-PPU configurations. After power/reset,
    // writes to this register are ignored until the first pre-render scanline.
    union {
        struct {
            uint8_t nametable_x_ : 1; // Nametable select for horizontal mirroring (0: $2000, 1: $2400)
            uint8_t nametable_y_ : 1; // Nametable select for vertical mirroring (0: $2000, 1: $2800)
            uint8_t increment_mode_ : 1; // 0: add 1, going across; 1: add 32, going down)
            uint8_t sprite_pattern_table_ : 1;
            // Sprite pattern table address for 8x8 sprites (0: $0000; 1: $1000; ignored in 8x16 mode)
            uint8_t background_pattern_table_ : 1; // Background pattern table address (0: $0000; 1: $1000)
            uint8_t sprite_size_ : 1; // 0: 8x8 pixels, 1: 8x16 pixels
            uint8_t slave_mode_ : 1; // Unused on the NES
            uint8_t enable_nmi_ : 1; // Vblank NMI enable (0: off, 1: on)
        };

        uint8_t value_;
    } ctrl_;

    // PPUMASK (the "mask" register) controls the rendering of sprites and backgrounds, as well as color effects.
    // After power/reset, writes to this register are ignored until the first pre-render scanline.
    // Most commonly, PPUMASK is set to $00 outside of gameplay to allow transferring a large amount of data to VRAM,
    // and $1E during gameplay to enable all rendering with no color effects.
    union {
        struct {
            uint8_t grayscale_ : 1; // Grayscale mode (0: off, 1: on)
            uint8_t show_background_leftmost_8px_ : 1; // Show leftmost 8px of background (0: off, 1: on)
            uint8_t show_sprites_leftmost_8px_ : 1; // Show leftmost 8px of sprites (0: off, 1: on)
            uint8_t show_background_ : 1; // Show background (0: off, 1: on)
            uint8_t show_sprites_ : 1; // Show sprites (0: off, 1: on)
            uint8_t emphasize_red_ : 1; // Emphasize red (green on PAL/Dendy) component (0: off, 1: on)
            uint8_t emphasize_green_ : 1; // Emphasize green (red on PAL/Dendy) component (0: off, 1: on)
            uint8_t emphasize_blue_ : 1; // Emphasize blue component (0: off, 1: on)
        };

        uint8_t value_;
    } mask_;

    // PPUSTATUS (the "status" register) reflects the state of rendering-related events and is primarily used for timing.
    // The three flags in this register are automatically cleared on dot 1 of the prerender scanline;
    // Reading this register has the side effect of clearing the PPU's internal w register.
    // It is commonly read before writes to PPUSCROLL and PPUADDR to ensure the writes occur in the correct order.
    union {
        struct {
            uint8_t unused_ : 5; // Unused bits, always read as 0
            uint8_t sprite_overflow_ : 1; // Sprite overflow (0: no overflow, 1: overflow occurred)
            uint8_t sprite_0_hit_ : 1; // Sprite 0 hit (0: no hit, 1: hit occurred)
            uint8_t vertical_blank_ : 1; // Vertical blank (0: not in vblank, 1: in vblank)
        };

        uint8_t value_;
    } status_;

private:
    static constexpr Pixel kPalette[64] = {
        [0x00] = {84, 84, 84}, [0x01] = {0, 30, 116}, [0x02] = {8, 16, 144}, [0x03] = {48, 0, 136},
        [0x04] = {68, 0, 100}, [0x05] = {92, 0, 48}, [0x06] = {84, 4, 0}, [0x07] = {60, 24, 0},
        [0x08] = {32, 42, 0}, [0x09] = {8, 58, 0}, [0x0A] = {0, 64, 0}, [0x0B] = {0, 60, 0},
        [0x0C] = {0, 50, 60}, [0x0D] = {0, 0, 0}, [0x0E] = {0, 0, 0}, [0x0F] = {0, 0, 0},
        [0x10] = {152, 150, 152}, [0x11] = {8, 76, 196}, [0x12] = {48, 50, 236}, [0x13] = {92, 30, 228},
        [0x14] = {136, 20, 176}, [0x15] = {160, 20, 100}, [0x16] = {152, 34, 32}, [0x17] = {120, 60, 0},
        [0x18] = {84, 90, 0}, [0x19] = {40, 114, 0}, [0x1A] = {8, 124, 0}, [0x1B] = {0, 118, 40},
        [0x1C] = {0, 102, 120}, [0x1D] = {0, 0, 0}, [0x1E] = {0, 0, 0}, [0x1F] = {0, 0, 0},
        [0x20] = {236, 238, 236}, [0x21] = {76, 154, 236}, [0x22] = {120, 124, 236}, [0x23] = {176, 98, 236},
        [0x24] = {228, 84, 236}, [0x25] = {236, 88, 180}, [0x26] = {236, 106, 100}, [0x27] = {212, 136, 32},
        [0x28] = {160, 170, 0}, [0x29] = {116, 196, 0}, [0x2A] = {76, 208, 32}, [0x2B] = {56, 204, 108},
        [0x2C] = {56, 180, 204}, [0x2D] = {60, 60, 60}, [0x2E] = {0, 0, 0}, [0x2F] = {0, 0, 0},
        [0x30] = {236, 238, 236}, [0x31] = {168, 204, 236}, [0x32] = {188, 188, 236}, [0x33] = {212, 178, 236},
        [0x34] = {236, 174, 236}, [0x35] = {236, 174, 212}, [0x36] = {236, 180, 176}, [0x37] = {228, 196, 144},
        [0x38] = {204, 210, 120}, [0x39] = {180, 222, 120}, [0x3A] = {168, 226, 144}, [0x3B] = {152, 226, 180},
        [0x3C] = {160, 214, 228}, [0x3D] = {160, 162, 160}, [0x3E] = {0, 0, 0}, [0x3F] = {0, 0, 0}
    };

    uint8_t name_table_[2][32 * 32] = {}; // 2 name tables, each 1024 (32*32) bytes
    std::vector<Pixel> framebuffer_ = std::vector<Pixel>(kWidth * kHeight, {0, 0, 0});
    uint16_t scanline_ = 0; // Current scanline (horizontal, 262 total)
    uint16_t cycle_ = 0; // Current cycle (vertical, 341 cycles per scanline)
    bool write_toggle_ = false;
    uint8_t data_buffer_ = 0x00;
    uint16_t oam_address_ = 0x0000;
    uint8_t palette_written_to[32];

};
