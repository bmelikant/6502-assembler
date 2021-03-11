#ifndef _6502_ASM_H
#define _6502_ASM_H

#include <string>
#include <map>

#include <cstdint>

struct AssemblerState;

class Assembler6502 {
public:

    static void assemble(std::string line);
    static void clear();
    
private:
    static AssemblerState *as;
    static void init();
};

#endif