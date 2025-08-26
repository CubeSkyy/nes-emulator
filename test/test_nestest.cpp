#include <gtest/gtest.h>

#include "bus.h"
#include "cpu.h"        // Adjust path according to your project structure
#include "log/logging.h"

class NestestRomTest : public ::testing::Test {
protected:
    static CPU* cpu;
    static Bus* bus;

    // Before all tests
    static void SetUpTestSuite() {
        cpu = new CPU();
        bus = new Bus(cpu, nullptr);
    }

    // Runs once after all tests
    static void TearDownTestSuite() {
        delete bus;
        bus = nullptr;
        delete cpu;
        cpu = nullptr;
    }
};

CPU* NestestRomTest::cpu = nullptr;
Bus* NestestRomTest::bus = nullptr;


TEST_F(NestestRomTest, LoadAndRunNestest) {
    std::string rom_path = "roms/nes-testroms/other/nestest.nes";

    ASSERT_TRUE(bus->LoadCartridge(rom_path));
    cpu->set_PC(0xC000); // Automatic test PC

    while (cpu->SP() != 0xFF) {
        cpu->StepInstruction();
    }

    std::cout << "Result: 0x02: 0x" << Logging::ToHex(cpu->Read(0x02)) << " 0x03: 0x" <<
        Logging::ToHex(cpu->Read(0x03)) << std::endl;
}
