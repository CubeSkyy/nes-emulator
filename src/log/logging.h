#ifndef LOGGING_H
#define LOGGING_H
#include <cstdint>
#include "cpu.h"

//#define LOGMODE // Uncomment to enable logging

class Logging {
public:
    static std::string CreateNeslogLine(const CPU& cpu);
    static std::string CreateDisassemblyLine(const CPU& cpu, uint16_t pc);
    static std::string ToHex(uint8_t val);
    static uint8_t GetOperationLength(void (CPU::*addr_mode)());

    static std::string Trim(const std::string& s);
private:
    static std::string GetOperandString(const CPU::Operation& entry, uint8_t op1, uint8_t op2, uint16_t pc);
    static bool HasValue(void (CPU::*addr_mode)());
    static uint16_t GetEffectiveAddress(const CPU& cpu, void (CPU::*addr_mode)(), uint8_t op1, uint8_t op2);
    static bool IsUndocumentedNop(uint8_t opcode);
};


#endif //LOGGING_H
