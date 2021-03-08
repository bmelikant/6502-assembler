#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <regex>

#include <cstdint>

#include <iostream>

#include "ltokenizer.h"

enum ArgumentType {
    NoArgument, ImmediateValue,
    ZeroPage, ZeroPageX,
    Absolute, AbsoluteX, AbsoluteY,
    IndirectX, IndirectY, Invalid
};

struct Opcode {
    uint8_t opcode;            // 8 bit opcodes
    ArgumentType argType;   // type of argument this opcode encodes
    uint8_t size;              // number of bytes encoded by this opcode

    bool operator ==(const Opcode& opcode) {
        return (this->opcode == opcode.opcode &&
            this->argType == opcode.argType &&
            this->size == opcode.size);
    }
};

using directiveMethod = void (*)();

static const std::map<std::string, directiveMethod> directives = {
    { ".db", nullptr },
    { ".dw", nullptr },
    { ".org", nullptr },
    { ".times", nullptr }
};

static const std::map<std::string, std::vector<Opcode>> opcodes = {
    { 
        "adc", {{ 0x69, ImmediateValue, 2 }, { 0x65, ZeroPage, 2 }, { 0x75, ZeroPageX, 2 }, { 0x6D, Absolute, 3 }, 
            { 0x7D, AbsoluteX, 3 },  { 0x79, AbsoluteY, 3 }, { 0x61, IndirectX, 2 }, { 0x71, IndirectY, 2 }}
    },
    { 
        "nop", {{ 0xEA, NoArgument, 1 }}
    }
};

static const Opcode IllegalOpcode = { 0xFF, NoArgument, 0 };

// assembler internal data
uint16_t origin = 0, current_offset = 0;
size_t lineNumber = 0;
std::map<std::string, uint16_t> symtable;
std::vector<uint8_t> assembledCode;
bool successful = true;

// pattern-matching routine declarations
bool isOpcode(std::string token);
bool isDirective(std::string token);
bool isOpcodeArgument(std::string token);
bool isImmediate(std::string token);
bool isZeroPage(std::string token);
bool isZeroPageXOffset(std::string token);
bool isAbsoluteMem(std::string token);
bool isAbsoluteMemXOffset(std::string token);
bool isAbsoluteMemYOffset(std::string token);
bool isIndirectMemXOffset(std::string token);
bool isIndirectMemYOffset(std::string token);
bool isLabel(std::string token);

bool opcodeRequiresArgument(std::string opcode);
bool opcodeAcceptsArgument(std::string opcode);

Opcode getOpcode(std::string opcode, std::string argument);
ArgumentType getOpcodeArgumentType(std::string argument);

void error(std::string errmsg);

void doOpcode(std::string opcode, LineTokenizer lt, int pass);
void doOpcodeArgument(std::string opcode, std::string argument, LineTokenizer lt, int pass);

// see API note in parser.h
bool assemble(std::istream fi) {
    return true;
}

// see API note in parser.h
void assemble(std::string line, int pass) {
    LineTokenizer lt(line);
    std::string token = lt.nextToken();

    // empty token is returned for blank line
    if (!token.empty()) {
        if (isOpcode(token)) {
            doOpcode(token, lt, pass);
        }
    }

    lineNumber++;
}

void doOpcode(std::string opcode, LineTokenizer lt, int pass) {
    std::string token = lt.nextToken();

#ifdef DEBUG
    std::cout << "current opcode is " << opcode << std::endl;
    std::cout << "the opcode " << ((opcodeRequiresArgument(opcode)) ? "does" : "does not") << " require an argument" << std::endl;
    std::cout << "the opcode " << ((opcodeAcceptsArgument(opcode)) ? "does" : "does not") << " accept an argument" << std::endl;
    std::cout << "the next token " << ((token.empty()) ? "is empty" : "is not empty") << std::endl;
#endif

    // check if we need an argument
    if (opcodeRequiresArgument(opcode) && token.empty()) {
        error("Invalid combination of opcode and operands");
    } else if (opcodeAcceptsArgument(opcode) && !token.empty()) {
        doOpcodeArgument(opcode, token, lt, pass);
    } else if (!opcodeAcceptsArgument(opcode) && !token.empty()) {
        error("Invalid combination of opcode and operands");
    } else {
        Opcode op = getOpcode(opcode, "");
        if (op == IllegalOpcode) {
            error("Invalid combination of opcode and operands");
        } else {
            assembledCode.push_back(op.opcode);
            current_offset += op.size;
        }
    }
}

void doOpcodeArgument(std::string opcode, std::string argument, LineTokenizer lt, int pass) {
    Opcode op = getOpcode(opcode, argument);
    if (op == IllegalOpcode) {
        error("Invalid combination of opcode and operands");
    } else {
        std::cout << "opcode " << opcode << " with argument " << argument << std::endl;
        std::cout << "assembled opcode is " << (int) op.opcode << ", argument type is " << op.argType << ", " << (int) op.size << " byte instruction" << std::endl;
        current_offset += op.size;
    }
}

// see API note in assembler.h
std::vector<uint8_t> getObjectFile() { return assembledCode; }
bool isSuccessfulAssembly() { return successful; }

/** Non-API functions **/

extern std::string tolower(std::string s);

/**
 * isOpcode(): check to see if the given token is an opcode
 */
bool isOpcode(std::string token) {
    token = tolower(token);
    return (opcodes.find(token) != opcodes.end());
}

/**
 * opcodeRequiresArgument(): returns true if the given opcode requires an argument to be present
 */
bool opcodeRequiresArgument(std::string opcode) {
    opcode = tolower(opcode);
    std::vector<Opcode> opcodeData = opcodes.find(opcode)->second;
    for (size_t i = 0; i < opcodeData.size(); i++) {
        if (opcodeData[i].argType == NoArgument) {
            return false;
        }
    }

    return true;
}

/**
 * opcodeAcceptsArgument(): returns true if an opcode can accept arguments
 */
bool opcodeAcceptsArgument(std::string opcode) {
    opcode = tolower(opcode);
    std::vector<Opcode> opcodeData = opcodes.find(opcode)->second;
    for (size_t i = 0; i < opcodeData.size(); i++) {
        if (opcodeData[i].argType != NoArgument) return true;
    }
    return false;
}

Opcode getOpcode(std::string opcode, std::string argument) {
    opcode = tolower(opcode);
    std::vector<Opcode> opcodeData = opcodes.find(opcode)->second;
    ArgumentType searchType = getOpcodeArgumentType(argument);

    if (searchType != Invalid) {
        for (size_t i = 0; i < opcodeData.size(); i++) {
            if (opcodeData[i].argType == searchType) return opcodeData[i];
        }
    }

    return IllegalOpcode;
}

const char *ImmediateValueRegex = "^#\\$[0-9a-f]{2}$";
const char *ZeroPageRegex = "^\\$[0-9a-f]{2}$";
const char *ZeroPageXOffsetRegex = "^\\$[0-9a-f]{2},x$";
const char *AbsoluteRegex = "^\\$[0-9a-f]{4}$";
const char *AbsoluteXOffsetRegex = "^\\$[0-9a-f]{4},x$";
const char *AbsoluteYOffsetRegex = "^\\$[0-9a-f]{4},y$";
const char *IndirectXOffsetRegex = "^\\(\\$[0-9a-f]{2},x\\)$";
const char *IndirectYOffsetRegex = "^\\(\\$[0-9a-f]{2}\\),y$";

/**
 * try to work out the correct argument type based on the pattern
 */
ArgumentType getOpcodeArgumentType(std::string argument) {
    if (argument.empty()) return NoArgument;
    if (std::regex_match(argument, std::regex(ImmediateValueRegex))) return ImmediateValue;
    if (std::regex_match(argument, std::regex(ZeroPageRegex))) return ZeroPage;
    if (std::regex_match(argument, std::regex(ZeroPageXOffsetRegex))) return ZeroPageX;
    if (std::regex_match(argument, std::regex(AbsoluteRegex))) return Absolute;
    if (std::regex_match(argument, std::regex(AbsoluteXOffsetRegex))) return AbsoluteX;
    if (std::regex_match(argument, std::regex(AbsoluteYOffsetRegex))) return AbsoluteY;
    if (std::regex_match(argument, std::regex(IndirectXOffsetRegex))) return IndirectX;
    if (std::regex_match(argument, std::regex(IndirectYOffsetRegex))) return IndirectY;

    return Invalid;
}

void error(std::string errmsg) {
    std::cout << "Line " << lineNumber << " - error: " << errmsg << std::endl;
    successful = false;
}