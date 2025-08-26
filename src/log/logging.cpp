#include "logging.h"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <unordered_set>
#include <algorithm>
#include "cpu.h"

uint8_t Logging::GetOperationLength(void (CPU::*addr_mode)()) {
    if (addr_mode == &CPU::ADR_IMM ||
        addr_mode == &CPU::ADR_ZP0 ||
        addr_mode == &CPU::ADR_ZPX ||
        addr_mode == &CPU::ADR_ZPY ||
        addr_mode == &CPU::ADR_REL ||
        addr_mode == &CPU::ADR_IZX ||
        addr_mode == &CPU::ADR_IZY) {
        return 2;
    }

    if (addr_mode == &CPU::ADR_ABS ||
        addr_mode == &CPU::ADR_ABX ||
        addr_mode == &CPU::ADR_ABY ||
        addr_mode == &CPU::ADR_IND) {
        return 3;
    }

    return 1;
}

std::string Logging::Trim(const std::string& s) {
    auto view = std::string_view(s);
    auto not_space = [](const unsigned char ch) { return !std::isspace(ch); };
    const auto start = std::find_if(view.begin(), view.end(), not_space);
    const auto end = std::find_if(view.rbegin(), view.rend(), not_space).base();
    if (start >= end) return "";
    return {start, end};
}

std::string Logging::GetOperandString(const CPU::Operation& entry, const uint8_t op1, const uint8_t op2,
                                      const uint16_t pc) {
    std::ostringstream operand_stream;
    if (entry.addr_mode_ == &CPU::ADR_IMP &&
        (entry.name_ == "LSR" || entry.name_ == "ASL" || entry.name_ == "ROL" || entry.name_ == "ROR"))
        operand_stream << "A";
    else if (entry.addr_mode_ == &CPU::ADR_IMM)
        operand_stream << "#$" << ToHex(op1);
    else if (entry.addr_mode_ == &CPU::ADR_ZP0)
        operand_stream << "$" << ToHex(op1);
    else if (entry.addr_mode_ == &CPU::ADR_ZPX)
        operand_stream << "$" << ToHex(op1) << ",X";
    else if (entry.addr_mode_ == &CPU::ADR_ZPY)
        operand_stream << "$" << ToHex(op1) << ",Y";
    else if (entry.addr_mode_ == &CPU::ADR_REL)
        operand_stream << "$" << std::setw(4) << std::setfill('0') << std::hex << std::uppercase
            << static_cast<uint16_t>(pc + 2 + static_cast<int8_t>(op1));
    else if (entry.addr_mode_ == &CPU::ADR_IZX)
        operand_stream << "($" << ToHex(op1) << ",X)";
    else if (entry.addr_mode_ == &CPU::ADR_IZY)
        operand_stream << "($" << ToHex(op1) << "),Y";
    else if (entry.addr_mode_ == &CPU::ADR_ABS)
        operand_stream << "$" << ToHex(op2) << ToHex(op1);
    else if (entry.addr_mode_ == &CPU::ADR_ABX)
        operand_stream << "$" << ToHex(op2) << ToHex(op1) << ",X";
    else if (entry.addr_mode_ == &CPU::ADR_ABY)
        operand_stream << "$" << ToHex(op2) << ToHex(op1) << ",Y";
    else if (entry.addr_mode_ == &CPU::ADR_IND)
        operand_stream << "($" << ToHex(op2) << ToHex(op1) << ")";
    else
        operand_stream << "";

    return operand_stream.str();
}

bool Logging::HasValue(void (CPU::*addr_mode)()) {
    if (
        addr_mode == &CPU::ADR_ZP0 ||
        addr_mode == &CPU::ADR_ZPX ||
        addr_mode == &CPU::ADR_ZPY ||
        addr_mode == &CPU::ADR_IZX ||
        addr_mode == &CPU::ADR_IZY ||
        addr_mode == &CPU::ADR_ABS ||
        addr_mode == &CPU::ADR_ABX ||
        addr_mode == &CPU::ADR_ABY
    ) {
        return true;
    }
    return false;
}

uint16_t Logging::GetEffectiveAddress(const CPU& cpu, void (CPU::*addr_mode)(), const uint8_t op1, const uint8_t op2) {
    if (addr_mode == &CPU::ADR_ZP0) {
        return op1;
    }
    if (addr_mode == &CPU::ADR_ZPX) {
        return (op1 + cpu.X()) & 0xFF;
    }
    if (addr_mode == &CPU::ADR_ZPY) {
        return (op1 + cpu.Y()) & 0xFF;
    }
    if (addr_mode == &CPU::ADR_IZX) {
        const uint8_t base = (op1 + cpu.X()) & 0xFF;
        return cpu.Read(base) | (cpu.Read((base + 1) & 0xFF) << 8);
    }
    if (addr_mode == &CPU::ADR_IZY) {
        const uint8_t lo = cpu.Read(op1);
        const uint8_t hi = cpu.Read((op1 + 1) & 0xFF);
        return ((hi << 8) | lo) + cpu.Y();
    }
    if (addr_mode == &CPU::ADR_ABS) {
        return (op2 << 8) | op1;
    }
    if (addr_mode == &CPU::ADR_ABX) {
        return ((op2 << 8) | op1) + cpu.X();
    }
    if (addr_mode == &CPU::ADR_ABY) {
        return ((op2 << 8) | op1) + cpu.Y();
    }
    return -1;
}

bool Logging::IsUndocumentedNop(const uint8_t opcode) {
    static const std::unordered_set<uint8_t> unofficial_nops = {
        0x04, 0x44, 0x64, 0x0C,
        0x14, 0x34, 0x54, 0x74, 0xD4, 0xF4,
        0x1A, 0x3A, 0x5A, 0x7A, 0xDA, 0xFA,
        0x80,
        0x82, 0x89, 0xC2, 0xE2,
        0xCB,
        0x1C, 0x3C, 0x5C, 0x7C, 0xDC, 0xFC
    };
    return unofficial_nops.count(opcode) > 0;
}

// C++
std::string Logging::CreateNeslogLine(const CPU& cpu) {
    const uint16_t pc = cpu.PC();
    std::string disasm = CreateDisassemblyLine(cpu, pc);

    std::ostringstream line_stream;
    line_stream << disasm
        << "A:" << ToHex(cpu.A()) << " "
        << "X:" << ToHex(cpu.X()) << " "
        << "Y:" << ToHex(cpu.Y()) << " "
        << "P:" << ToHex(cpu.P()) << " "
        << "SP:" << ToHex(cpu.SP()) << " "
        << "CYC:" << std::dec << cpu.TotalCycles() << "\n";

    std::string line = line_stream.str();

    std::ofstream neslog_file("neslog_output.log", std::fstream::app);
    neslog_file << std::string(line);
    neslog_file.close();

    return {line};
}

std::string Logging::CreateDisassemblyLine(const CPU& cpu, uint16_t pc) {
    const uint8_t opcode = cpu.Read(pc);
    const uint8_t op1 = cpu.Read(pc + 1);
    const uint8_t op2 = cpu.Read(pc + 2);
    const auto& entry = CPU::GetOpcodeEntry(opcode);
    const uint8_t op_len = GetOperationLength(entry.addr_mode_);

    std::ostringstream opbytes_stream;
    if (op_len == 1)
        opbytes_stream << ToHex(opcode);
    else if (op_len == 2)
        opbytes_stream << ToHex(opcode) << " " << ToHex(op1);
    else
        opbytes_stream << ToHex(opcode) << " " << ToHex(op1) << " " << ToHex(op2);
    std::string opbytes = opbytes_stream.str();

    std::string operand_str;
    std::string value_str;

    // Special formatting for ($nn,X) and ($nn),Y
    if (entry.addr_mode_ == &CPU::ADR_IZX) {
        uint8_t zp_addr = (op1 + cpu.X()) & 0xFF;
        uint16_t eff_addr = cpu.Read(zp_addr) | (cpu.Read((zp_addr + 1) & 0xFF) << 8);
        uint8_t value = cpu.Read(eff_addr);
        operand_str = "($" + ToHex(op1) + ",X) @ " + ToHex(zp_addr) + " = " + ToHex((eff_addr >> 8) & 0xFF) +
            ToHex(eff_addr & 0xFF) + " = " + ToHex(value);
    }
    else if (entry.addr_mode_ == &CPU::ADR_IZY) {
        uint8_t base_lo = cpu.Read(op1);
        uint8_t base_hi = cpu.Read((op1 + 1) & 0xFF);
        uint16_t base_addr = (base_hi << 8) | base_lo;
        uint16_t eff_addr = base_addr + cpu.Y();
        uint8_t value = cpu.Read(eff_addr);
        operand_str = "($" + ToHex(op1) + "),Y = " +
            ToHex((base_addr >> 8) & 0xFF) + ToHex(base_addr & 0xFF) +
            " @ " +
            ToHex((eff_addr >> 8) & 0xFF) + ToHex(eff_addr & 0xFF) +
            " = " + ToHex(value);
    }
    else if (entry.name_ == "JMP" && entry.addr_mode_ == &CPU::ADR_IND) {
        uint16_t ptr = (op2 << 8) | op1;
        uint8_t lo = cpu.Read(ptr);
        uint8_t hi = cpu.Read((ptr & 0xFF00) | ((ptr + 1) & 0xFF)); // 6502 bug emulation
        uint16_t target = (hi << 8) | lo;
        operand_str = "($" + ToHex(op2) + ToHex(op1) + ") = " + ToHex((target >> 8) & 0xFF) + ToHex(target & 0xFF);
    }
    else if (entry.addr_mode_ == &CPU::ADR_ABX || entry.addr_mode_ == &CPU::ADR_ABY) {
        operand_str = GetOperandString(entry, op1, op2, pc);
        uint16_t base_addr = (op2 << 8) | op1;
        uint16_t eff_addr = (entry.addr_mode_ == &CPU::ADR_ABX) ? (base_addr + cpu.X()) : (base_addr + cpu.Y());
        uint8_t value = cpu.Read(eff_addr);
        operand_str += " @ " + ToHex((eff_addr >> 8) & 0xFF) + ToHex(eff_addr & 0xFF);
        value_str = "= " + ToHex(value);
    }
    else if (entry.addr_mode_ == &CPU::ADR_ZPX || entry.addr_mode_ == &CPU::ADR_ZPY) {
        operand_str = GetOperandString(entry, op1, op2, pc);
        uint16_t eff_addr = (entry.addr_mode_ == &CPU::ADR_ZPX)
                                ? ((op1 + cpu.X()) & 0xFF)
                                : ((op1 + cpu.Y()) & 0xFF);
        uint8_t value = cpu.Read(eff_addr);
        operand_str += " @ " + ToHex(eff_addr) + " = " + ToHex(value);
        value_str = ""; // already included
    }
    else {
        operand_str = GetOperandString(entry, op1, op2, pc);
        uint16_t addr = GetEffectiveAddress(cpu, entry.addr_mode_, op1, op2);
        if (HasValue(entry.addr_mode_) && (
            entry.name_ == "LDA" || entry.name_ == "STA" ||
            entry.name_ == "LDX" || entry.name_ == "STX" ||
            entry.name_ == "LDY" || entry.name_ == "STY" ||
            entry.name_ == "INC" || entry.name_ == "DEC" ||
            entry.name_ == "CMP" || entry.name_ == "CPX" || entry.name_ == "CPY" ||
            entry.name_ == "AND" || entry.name_ == "ORA" || entry.name_ == "EOR" ||
            entry.name_ == "SBC" || entry.name_ == "ADC" || entry.name_ == "BIT" ||
            entry.name_ == "LSR" || entry.name_ == "ASL" || entry.name_ == "ROL" || entry.name_ == "ROR"
        )) {
            value_str = "= " + ToHex(cpu.Read(addr));
        }
    }

    std::string mnemonic_str = IsUndocumentedNop(opcode)
                                   ? ("*NOP $" + (op_len == 3
                                                      ? ToHex(op2) + ToHex(op1) + " = " + ToHex(
                                                          cpu.Read((op2 << 8) | op1))
                                                      : ToHex(op1) + " = " + ToHex(cpu.Read(op1))))
                                   : (std::string(entry.name_) + " " + operand_str + " " + value_str);

    std::ostringstream line_stream;
    line_stream << std::left << std::setfill(' ') << std::setw(6) << ToHex(pc >> 8) + ToHex(pc)
        << std::setw(10) << opbytes
        << std::setw(32) << mnemonic_str;
    return line_stream.str();
}

// Helper to format hex
std::string Logging::ToHex(const uint8_t val) {
    char buf[3];
    snprintf(buf, sizeof(buf), "%02X", val);
    return buf;
}
