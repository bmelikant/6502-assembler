#ifndef _6502_OPCODE_H
#define _6502_OPCODE_H

#include <string>

struct Opcode {
    std::string mnemonic;
    std::string addrMode;
};

extern Opcode opcodeMatrix[0x0f][0x0f];

#endif