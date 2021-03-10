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
    ZeroPage, ZeroPageWithXOffset,
    Absolute, AbsoluteWithXOffset, AbsoluteWithYOffset,
    IndexedIndirect, IndirectIndexed, Invalid,
    Label, Indirect, LabelRelative, LabelIndirect, Accumulator
};


struct Opcode {
    uint8_t opcode;                     // 8 bit opcodes
    ArgumentType argType;               // type of argument this opcode encodes
    uint8_t size;                       // number of bytes encoded by this opcode

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
    { "adc", {{ 0x69, ImmediateValue, 2 }, { 0x65, ZeroPage, 2 }, { 0x75, ZeroPageWithXOffset, 2 }, { 0x6D, Absolute, 3 }, 
        { 0x7D, AbsoluteWithXOffset, 3 },  { 0x79, AbsoluteWithYOffset, 3 }, { 0x61, IndexedIndirect, 2 }, { 0x71, IndirectIndexed, 2 }}
    },
    { "and", {{0x29, ImmediateValue, 2 }, { 0x25, ZeroPage, 2 }, { 0x35, ZeroPageWithXOffset, 2 }, { 0x2D, Absolute, 3 },
        { 0x3D, AbsoluteWithXOffset, 3 }, { 0x39, AbsoluteWithYOffset, 3 }, { 0x21, IndexedIndirect, 2 }, { 0x31, IndirectIndexed, 2 }}
    },
    { "asl", {{ 0x0A, Accumulator, 1 }, { 0x06, ZeroPage, 2 }, { 0x16, ZeroPageWithXOffset, 2 }, { 0x0E, Absolute, 3 }, { 0x1E, AbsoluteWithXOffset, 3 }}},
    { "bit", {{ 0x24, ZeroPage, 2 }, { 0x2C, Absolute, 3 }}},
    { "bpl", {{ 0x10, LabelRelative, 2 }, { 0x10, ZeroPage, 2 }}},
    { "bmi", {{ 0x30, LabelRelative, 2 }, { 0x30, ZeroPage, 2 }}},
    { "bvc", {{ 0x50, LabelRelative, 2 }, { 0x50, ZeroPage, 2 }}},
    { "bvs", {{ 0x70, LabelRelative, 2 }, { 0x70, ZeroPage, 2 }}},
    { "bcc", {{ 0x90, LabelRelative, 2 }, { 0x90, ZeroPage, 2 }}},
    { "bcs", {{ 0xB0, LabelRelative, 2 }, { 0xB0, ZeroPage, 2 }}},
    { "bne", {{ 0xD0, LabelRelative, 2 }, { 0xD0, ZeroPage, 2 }}},
    { "beq", {{ 0xF0, LabelRelative, 2 }, { 0xF0, ZeroPage, 2 }}},
    { "jmp", {{ 0x4C, Label, 3 }, { 0x4C, Absolute, 3 }, { 0x6C, Indirect, 3 }, { 0x6C, LabelIndirect, 3 }}},
    { "nop", {{ 0xEA, NoArgument, 1 }}}
};

static const Opcode IllegalOpcode = { 0xFF, NoArgument, 0 };

static const std::map<ArgumentType, const char *> typeRegexMap = {
    { ImmediateValue, "^#\\$([0-9a-f]{1,2})$" },
    { ZeroPage,  "^\\$([0-9a-f]{1,2})$" },
    { ZeroPageWithXOffset, "^\\$([0-9a-f]{1,2}),x$" },
    { Absolute,  "^\\$([0-9a-f]{3,4})$" },
    { AbsoluteWithXOffset, "^\\$([0-9a-f]{3,4}),x$" },
    { AbsoluteWithYOffset, "^\\$([0-9a-f]{3,4}),y$" },
    { IndexedIndirect, "^\\(\\$([0-9a-f]{1,2}),x\\)$" },
    { IndirectIndexed, "^\\(\\$([0-9a-f]{1,2})\\),y$" },
    { Indirect,  "^\\(\\$([0-9a-f]{3,4})\\)$" },
    { Label, "^[0-9a-zA-z_]*$" },
    { LabelIndirect, "^\\(([0-9a-zA-z_]*)\\)$" }
};

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

bool opcodeRequiresArgument(std::string opcode);
bool opcodeAcceptsArgument(std::string opcode);

Opcode getOpcode(std::string opcode, std::string argument);
ArgumentType getOpcodeArgumentType(std::string argument);

void error(std::string errmsg);
void warn(std::string warning);

void doOpcode(std::string opcode, LineTokenizer lt, int pass);
void doOpcodeArgument(std::string opcode, std::string argument, LineTokenizer lt, int pass);

uint16_t getArgumentValue(Opcode op, std::string argument);
uint16_t getLabelAddress(std::string label, bool asOffsetFromCurrent);
uint16_t getLabelPosition(Opcode op, std::string argument);
std::string stripIndirectLabel(std::string label);

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

void insertSymbol(std::string symbol, uint16_t address) {
    symtable[symbol] = address;
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
        error("Illegal combination of opcode and operands");
    } else if (opcodeAcceptsArgument(opcode) && !token.empty()) {
        doOpcodeArgument(opcode, token, lt, pass);
    } else if (!opcodeAcceptsArgument(opcode) && !token.empty()) {
        error("Illegal combination of opcode and operands");
    } else {
        Opcode op = getOpcode(opcode, "");
        if (op == IllegalOpcode) {
            error("Illegal combination of opcode and operands");
        } else {
            if (pass == 2) assembledCode.push_back(op.opcode);
            current_offset += op.size;
        }
    }
}

void doOpcodeArgument(std::string opcode, std::string argument, LineTokenizer lt, int pass) {
    Opcode op = getOpcode(opcode, argument);
    if (op == IllegalOpcode) {
        error("Illegal combination of opcode and operands");
    } else {
#ifdef DEBUG
        std::cout << "opcode " << opcode << " with argument " << argument << std::endl;
        std::cout << "assembled opcode is " << (int) op.opcode << ", argument type is " << op.argType << ", " << (int) op.size << " byte instruction" << std::endl;
#endif
        if (pass == 2) assembledCode.push_back(op.opcode);

        // on the second pass, assemble the code and perform fixups if necessary
        if (op.argType == Accumulator) {
            // skip it. nothing to do here. these are single-byte opcodes that accept an argument. weird.
            #ifdef DEBUG
            std::cout << "argument type is accumulator. nothing else to do" << std::endl;
            #endif
        } else if ((op.argType == Label || op.argType == LabelRelative || op.argType == LabelIndirect)) {
            // strip away the () around indirect labels
            if (op.argType == LabelIndirect) argument = stripIndirectLabel(argument);

            if (pass == 2) {
                uint16_t labelAddress = getLabelAddress(argument, (op.argType == LabelRelative));

                // warn if an indirect jump crosses a page boundary
                if (op.opcode == 0x6C && (labelAddress &~ 0xff00) == 0xff) {
                    warn("Indirect jump reference crosses page boundary ");
                }

                uint8_t lowByte = (uint8_t) (labelAddress &~ 0xff00);
                uint8_t hiByte = (uint8_t) (labelAddress >> 8);

                assembledCode.push_back(lowByte);
                if (op.argType != LabelRelative) assembledCode.push_back(hiByte);
            }
        } else {
            uint16_t argumentValue = getArgumentValue(op, argument);
#ifdef DEBUG
            std::cout << "the argument value is " << argumentValue << std::endl;
#endif
            // encode the argument value into the assembly!
            uint8_t lowByte = (uint8_t) (argumentValue &~ 0xff00);
            uint8_t hiByte = (uint8_t) (argumentValue >> 8);

            if (pass == 2) {

                if (op.opcode == 0x6C && (argumentValue &~ 0xff00) == 0xff) {
                    warn("Indirect jump reference crosses page boundary");
                }
                // account for endianness
                assembledCode.push_back(lowByte);
                if (op.size == 3) assembledCode.push_back(hiByte);
            }
        }

        // TODO: check to see if this computation should come *before* or *after* relative label
        // computations. Do the "branch" instructions jump *before* or *after* incrementing PC?
        current_offset += op.size;
    }
}

uint16_t getArgumentValue(Opcode op, std::string argument) {
    std::map<ArgumentType, const char *>::const_iterator it = typeRegexMap.find(op.argType);
    if (it != typeRegexMap.end()) {
        std::smatch match;
        std::regex regex = std::regex(it->second);

        if (std::regex_search(argument, match, regex) == true && match.size() == 2) {
            std::string hexString = match.str(1);
            return (uint16_t) std::stoi(hexString, 0, 16);
        }
    }

    error("Internal assembler error");
    return 0;
}

uint16_t getLabelAddress(std::string label, bool asOffsetFromCurrent) {
    auto it = symtable.find(label);
    if (it != symtable.end()) {
        if (!asOffsetFromCurrent) {
            return it->second;
        } else {
            // formula for offset is target address - current offset. if the offset overflows
            // an 8-bit signed integer, it's an error
            int32_t destAddr = (int32_t) it->second;
            int32_t curAddr = (int32_t) current_offset;

            int32_t result = (destAddr - curAddr);
            #ifdef DEBUG
            std::cout << "relative jump is " << result << std::endl;
            #endif
            if (result >= -128 && result < 127) {
                // nasty conversion hack. convert first to uint8_t to chop off the top 24 bits
                // then convert to uint16_t. the result will be an 8-bit signed integer stuffed into a uint16_t
                // :)
                return (uint16_t) ((uint8_t) result);
            } else {
                error("Branch offset is out of range of byte");
            }
        }
    }

    error("Unknown label");
    return 0;
}

// see API note in assembler.h
std::vector<uint8_t> getObjectFile() { return assembledCode; }
bool isSuccessfulAssembly() { return successful; }
void prepareForSecondPass() { origin = current_offset = 0; lineNumber = 0; }

/** Non-API functions **/

extern std::string tolower(std::string s);

std::string stripIndirectLabel(std::string label) {
    std::regex regex = std::regex(typeRegexMap.find(LabelIndirect)->second);
    std::smatch match;

    if (std::regex_search(label, match, regex) == true) {
        return match.str(1);
    }

    return label;
}

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
            // relative labels are interchangable with absolute labels, since no instruction can contain both
            if (searchType == Label && opcodeData[i].argType == LabelRelative) return opcodeData[i];
            if (searchType == Label && argument == "a" && opcodeData[i].argType == Accumulator) return opcodeData[i];
        }
    }

    return IllegalOpcode;
}

/**
 * try to work out the correct argument type based on the pattern
 */
ArgumentType getOpcodeArgumentType(std::string argument) {
    if (argument.empty()) return NoArgument;

    std::map<ArgumentType, const char *>::const_iterator it;
    for (it = typeRegexMap.begin(); it != typeRegexMap.end(); it++) {
        if (std::regex_match(argument, std::regex(it->second))) {
            return it->first;
        }
    }

    return Invalid;
}

void error(std::string errmsg) {
    std::cout << "Line " << lineNumber << " - error: " << errmsg << std::endl;
    successful = false;
}

void warn(std::string warning) {
    std::cout << "Line " << lineNumber << " - warning: " << warning << std::endl;
}