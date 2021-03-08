#include <fstream>
#include <map>
#include <string>
#include <vector>

#include <cstdint>

#include "ltokenizer.h"

using Byte = uint8_t;
using Word = uint16_t;
using Addr = uint16_t;

enum ArgumentType {
    ImmediateValue,
    ZeroPage, ZeroPageX,
    Absolute, AbosluteX, AbsoluteY,
    IndirectX, IndirectY
};

struct Opcode {
    Byte opcode;            // 8 bit opcodes
    ArgumentType argType;   // type of argument this opcode encodes
    Byte size;              // number of bytes encoded by this opcode
};

using directiveMethod = void (*)();

static const std::map<std::string, directiveMethod> directives = {
    { ".db", nullptr },
    { ".dw", nullptr },
    { ".org", nullptr },
    { ".times", nullptr }
};

static const std::map<std::string, int> opcodes = {
};

std::map<std::string, Word> symtable;
std::vector<Byte> codeFile;

// assembler internal data
uint16_t origin = 0, current_offset = 0;

// see API note in parser.h
bool assemble(std::istream fi) {
    return true;
}

// see API note in parser.h
bool assemble(std::string line, int pass) {
    LineTokenizer lt(line);
    std::string token = lt.nextToken();

    // empty token is returned for blank line
    if (!token.empty()) {

    }

    return true;
}


