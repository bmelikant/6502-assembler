#ifndef _6502_ASM_H
#define _6502_ASM_H

#include <iostream>
#include <string>
#include <map>

#include <cstdint>

void assemble(std::string line);
void dumpSymbolTable();

void setOutFile(std::string ofname);
void setProgramStart(uint16_t origin);

void close();
void startNextPass();
bool isSuccessfulAssembly();

#endif