#include <gtest/gtest.h>

#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "log/logging.h"

class InstrV5RomTest : public ::testing::Test {
protected:
    static CPU* cpu;
    static PPU* ppu;
    static Bus* bus;

    // Before all tests
    static void SetUpTestSuite() {
        cpu = new CPU();
        ppu = new PPU();
        bus = new Bus(cpu, ppu);
    }

    // Runs once after all tests
    static void TearDownTestSuite() {
        delete bus;
        bus = nullptr;
        delete cpu;
        cpu = nullptr;
        delete ppu;
        ppu = nullptr;
    }
};

CPU* InstrV5RomTest::cpu = nullptr;
PPU* InstrV5RomTest::ppu = nullptr;
Bus* InstrV5RomTest::bus = nullptr;

TEST_F(InstrV5RomTest, LoadAndRunInstrV5) {
    std::vector<std::string> rom_files = {
        "01-basics.nes", // Passed
        "02-implied.nes", // Passed
        "03-immediate.nes", // FAIL (unofficial opcodes)
        "04-zero_page.nes", // Passed
        "05-zp_xy.nes", // Passed
        "06-absolute.nes", // Passed
        "07-abs_xy.nes", // Failing on timeout
        "08-ind_x.nes", // Passed
        "09-ind_y.nes", // Passed
        "10-branches.nes", // Passed
        "11-stack.nes", // Passed
        "12-jmp_jsr.nes", // Passed
        "13-rts.nes", // Passed
        "14-rti.nes", // Passed
        "15-brk.nes", // FAIL (no message)
        "16-special.nes" // FAIL (#5)
    };

    std::string full_output;

    for (const auto& rom_file : rom_files) {
        std::string rom_path = "roms/nes-testroms/instr_test-v5/rom_singles/" + rom_file;

        ASSERT_TRUE(bus->LoadCartridge(rom_path));

        bool seen_80 = false;
        while (true) {
            uint8_t val = cpu->Read(0x6000);
            if (cpu->TotalCycles() >= 5000000 || (seen_80 && val != 0x80)) break;
            if (val == 0x80) seen_80 = true;
            cpu->StepInstruction();
        }

        std::string output;
        uint16_t addr = 0x6004;
        while (true) {
            uint8_t byte = cpu->Read(addr++);
            if (byte == 0x00) break; // Null terminator
            output += static_cast<char>(byte);
        }

        std::cout << "[" << rom_file << ": " << cpu->TotalCycles() << "] " << output << std::endl;
        full_output += "[" + rom_file + ": " + std::to_string(cpu->TotalCycles()) + "] " + output + "\n";
    }

    std::cout << "=== Full Output ===" << std::endl;
    std::cout << full_output << std::endl;
}
