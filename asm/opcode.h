#ifndef _6502_OPCODE_H
#define _6502_OPCODE_H

#include <string>
#include <cstdint>

const uint8_t ILLEGAL_OPCODE = 0xf2;

struct Mnemonic {
    std::string mnemonic;
    std::string addrmode;

    bool operator==(const Mnemonic& op);
};

bool matchesOpcode(std::string token);
bool argumentIsLabelType(std::string token);
uint8_t findOpcode(std::string mnemonic, std::string argument);
std::string findAddressMode(std::string argument);
std::string stripLabel(std::string label);
int getInstructionSize(std::string addressMode);

#endif