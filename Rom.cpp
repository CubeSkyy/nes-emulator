//
// Created by miguelcoelho on 30-08-2022.
//

#include "Rom.h"
#include "fstream"
#include <iostream>
#include <utility>


using namespace std;

Rom::Rom() {

}

void Rom::LoadTestParams() {
    prg_count = 0;
    chr_count = 1;
    vertical_mirror = false;
    horizontal_mirror = false;
    battery_ram = false;
    trainer_present = false;
    mappertype_lower4bits = -1;
    mappertype_upper4bits = -1;

    prg_rom_size = -1;
    prg_rom_start = -1;
}


void Rom::LoadROM(const char *filename) {
    ifstream rom_file;
    rom_file.open(filename, ios::binary | ios::ate);

    if (rom_file.is_open()) {
        buffer_size = rom_file.tellg();
        buffer = new char[buffer_size];

        rom_file.seekg(0, ios::beg);
        rom_file.read(buffer, buffer_size);
        rom_file.close();

        // ---------------  Header -------------------
        // Validate NES header file
        if (buffer[0x00] != 0x4E || buffer[0x01] != 0x45 || buffer[0x02] != 0x53 || buffer[0x03] != 0x1A) {
            cout << "Header invalid. Did you load a .nes file?" << endl;
        }
        cout << "Header is Valid" << endl;

        // Get number of PRG and CHR Roms
        prg_count = buffer[0x04];
        chr_count = buffer[0x05];

        // Control bytes
        vertical_mirror = buffer[0x06] & 0x01u;
        horizontal_mirror = !vertical_mirror;
        battery_ram = (buffer[0x06] & 0x02u) >> 1u;
        trainer_present = (buffer[0x06] & 0x04u) >> 2u;
        mappertype_lower4bits = (buffer[0x06] & 0xF0u) >> 4u;

        if (((buffer[0x07] & 0x0Cu) >> 2u) == 0x10) {
            nes_version = 2;
        } else if (((buffer[0x07] & 0x0Cu) >> 2u) == 0x00
                   && ((buffer[0x07] & 0x01u) == 0x00)
                   && ((buffer[0x07] & 0x02u) >> 1u) == 0x00) {
            nes_version = 1;
        } else {
            cout << "Header is invalid. Nes version not recognized (Should be 1.0 or 2.0)" << endl;
        }

        mappertype_upper4bits = (buffer[0x07] & 0xF0u) >> 4u;

        // ROMs

        prg_rom_size = (PRG_ROM_PAGE_SIZE / 2) * prg_count;
        prg_rom_start = 16 + (trainer_present ? 512 : 0);

        prg_rom.insert(prg_rom.end(), &buffer[prg_rom_start], &buffer[prg_rom_start + prg_rom_size]);
        //copy(buffer + prg_rom_start, buffer + prg_rom_start + prg_rom_size, prg_rom);

        chr_rom_size = (CHR_ROM_PAGE_SIZE / 2) * chr_count;
        chr_rom_start = prg_rom_start + prg_rom_size;
        if (chr_rom_size > 0) {
            chr_rom.insert(chr_rom.end(), &buffer[chr_rom_start], &buffer[chr_rom_start + chr_rom_size]);
        }


        delete[] buffer;

        //spdlog::info("Rom Loaded.");
    }
}

void Rom::setChrRom(vector<uint8_t> chr_rom_in) {
    chr_rom = std::move(chr_rom_in);

}
