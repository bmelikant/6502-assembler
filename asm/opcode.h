#ifndef _6502_OPCODE_H
#define _6502_OPCODE_H

#include <string>
#include <cstdint>

struct Mnemonic {
    std::string mnemonic;
    std::string addrmode;

    bool operator==(const Mnemonic& op);
};

struct InstructionPacket {
    uint8_t opcode;
    uint16_t argument;
    int size;
    std::string label;
    bool isLabelType;
    bool isRelativeJump;

    bool operator==(const InstructionPacket& packet);
};

const uint8_t ILLEGAL_OPCODE = 0xf2;
extern InstructionPacket IllegalInstruction;

bool matchesOpcode(std::string token);
InstructionPacket buildInstruction(std::string mnemonic, std::string argument);

#endif