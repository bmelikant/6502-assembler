#ifndef PARSER_6502_H
#define PARSER_6502_H

#include <fstream>
#include <string>

/**
 * 
 */
bool assemble(std::istream fi);

/**
 * assemble(): read the given code line, tokenize the line, and attempt assembly
 * inputs: line - the line of code to assemble, pass - the current assembly pass
 * returns: true if the line successfully assembled; false otherwise
 * 
 * Pass one is used to build out the symbol table. Labels and their associated addresses
 * are gathered in pass one. Pass two performs code assembly and label fixups. If you are assembling
 * a single line of code, pass can be set to 0 (perform no address fixup)
 */
bool assemble(std::string line, int pass);

#endif