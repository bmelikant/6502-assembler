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
    IndirectX, IndirectY, Invalid,
    Label
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

static const std::map<ArgumentType, const char *> typeRegexMap = {
    { ImmediateValue, "^#\\$([0-9a-f]{1,2})$" },
    { ZeroPage, "^\\$([0-9a-f]{1,2})$" },
    { ZeroPageX,  "^\\$([0-9a-f]{1,2}),x$" },
    { Absolute, "^\\$([0-9a-f]{3,4})$" },
    { AbsoluteX, "^\\$([0-9a-f]{3,4}),x$" },
    { AbsoluteY, "^\\$([0-9a-f]{3,4}),y$" },
    { IndirectX, "^\\(\\$([0-9a-f]{1,2}),x\\)$" },
    { IndirectY, "^\\(\\$([0-9a-f]{1,2})\\),y$" }
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

void doOpcode(std::string opcode, LineTokenizer lt, int pass);
void doOpcodeArgument(std::string opcode, std::string argument, LineTokenizer lt, int pass);

uint16_t getArgumentValue(Opcode op, std::string argument);
uint16_t getLabelPosition(Opcode op, std::string argument);

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
            assembledCode.push_back(op.opcode);
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
        current_offset += op.size;

        // on the second pass, assemble the code and perform fixups if necessary
        uint16_t argumentValue = 0;
        if (op.argType == Label && pass == 2) {

        } else {
            argumentValue = getArgumentValue(op, argument);
#ifdef DEBUG
            std::cout << "the argument value is " << argumentValue << std::endl;
#endif
            // encode the argument value into the assembly!
            uint8_t lowByte = (uint8_t) (argumentValue &~ 0xff00);
            uint8_t hiByte = (uint8_t) (argumentValue >> 8);

            assembledCode.push_back(lowByte);
            if (op.size == 3) assembledCode.push_back(hiByte);
        }
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

    return Label;
}

void error(std::string errmsg) {
    std::cout << "Line " << lineNumber << " - error: " << errmsg << std::endl;
    successful = false;
}