#ifndef _6502_OPCODE_H
#define _6502_OPCODE_H

#include <string>
#include <cstdint>

const uint8_t ILLEGAL_OPCODE = 0xf2;

struct Opcode {
    std::string mnemonic;
    std::string addrmode;

    bool operator==(const Opcode& op);
};

bool matchesOpcode(std::string token);
bool argumentIsLabelType(std::string token);
uint8_t findOpcode(std::string mnemonic, std::string addrmode);
uint8_t findOpcode(Opcode);
std::string findAddressMode(std::string argument);
std::string stripLabel(std::string label);
int getInstructionSize(std::string addressMode);

#endif