#ifndef _6502_ASM_H
#define _6502_ASM_H

#include <string>
#include <map>

#include <cstdint>

void assemble(std::string line, int pass);
void dumpSymbolTable();

#endif