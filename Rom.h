//
// Created by miguelcoelho on 30-08-2022.
//

#ifndef NES_EMULATOR_ROM_H
#define NES_EMULATOR_ROM_H

#include <cstdint>
#include <random>
#include <vector>

using namespace std;

class Rom {
public:
    Rom();

    static const int PRG_ROM_PAGE_SIZE = 16384; // In bytes
    static const int CHR_ROM_PAGE_SIZE = 8192 ; // In bytes
    streampos buffer_size;
    char *buffer;
    int prg_count;
    int chr_count;
    int prg_rom_size;
    int chr_rom_size;
    uint8_t prg_rom_start;
    uint8_t chr_rom_start;
    // Control byte information
    bool vertical_mirror;
    bool horizontal_mirror;
    bool battery_ram; // Battery-backed ram at $6000-$7FFF
    bool trainer_present; // Indicates if a 512-byte trainer is at $7000-$71FF
    bool four_screen_vram; // Indicates if four-screen VRAM layout?
    uint8_t mappertype_lower4bits; //Four lower bits of ROM Mapper type
    uint8_t mappertype_upper4bits; //Four upper bits of ROM Mapper type
    int8_t nes_version; // Should be 1 or 2 depending on bits (3,2) of Control byte 2
    vector<uint8_t> prg_rom;
    vector<uint8_t> chr_rom;

    void LoadROM(char const *filename);


    void LoadTestParams();

    void setChrRom(vector<uint8_t> chr_rom_in);
};



#endif //NES_EMULATOR_ROM_H
