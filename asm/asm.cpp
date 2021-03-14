#include "opcode.h"
#include "ltokenizer.h"

#include <iostream>
#include <fstream>
#include <iomanip>
#include <map>
#include <regex>

using namespace std;

/** assembler variables **/
int pass = 1;
uint16_t offset = 0;
bool success = true, allowIllegalOpcodes = false;
size_t lineNo = 1;
map<string,uint16_t> symtable;
ofstream of;

void error(string msg) {
    // supress errors on first pass
    if (pass == 1) return;
    
    cerr << "Error (line " << dec << lineNo << "): " << msg << endl;
    success = false;
}

void warning(string msg) {
    // supress warnings on first pass
    if (pass == 1) return;

    cout << "Warning (line " << dec << lineNo << "): " << msg << endl;
}

void writeInstruction(InstructionPacket ip) {
    if (of.is_open()) {
        of.put(ip.opcode);
        if (ip.size > 1) of.put((uint8_t) (ip.argument & ~0xff00));
        if (ip.size > 2) of.put((uint8_t) (ip.argument >> 8));
    }
}

void printInstruction(InstructionPacket ip) {
    cout << "Opcode: $" << hex << setw(2) << (int) ip.opcode 
    << ", operand: $" << setw(4) << setfill('0') << ip.argument
    << ", size: " << setw(0) << setfill(' ') << ip.size
    << ", label: " << ip.label << endl;
}

const char *labelRegex = "^([0-9a-zA-Z_]*):?$";
bool matchesLabel(string token) {
    return (regex_match(token, regex(labelRegex)));
}

string stripLabel(string label) {
    regex re = regex(labelRegex);
    smatch match;

    if (regex_search(label, match, re) == true) return match.str(1);
    return label;
}

uint16_t getSymbolArgument(string label, bool relative) {
    cout << "looking for label " << label << endl;
    if (symtable.find(label) == symtable.end()) {
        error("Unknown label or mnemonic");
    } else {
        uint16_t address = symtable[label];
        if (relative) {
            int value = ((int) address) - ((int) offset);
            if (value < -128 || value > 127) {
                error("Relative jump out of range");
            }

            return (uint16_t) ((uint8_t) value);
        }

        return address;
    }

    return 0;
}

void doOpcode(string mnemonic, LineTokenizer lt) {
    string token = lt.nextToken();

    InstructionPacket ip = buildInstruction(mnemonic, token);
    if (ip == IllegalInstruction) {
        error("Illegal combination of opcode and operands");
    } else {
        if (pass == 2 && ip.isLabelType) {
            ip.argument = getSymbolArgument(ip.label, ip.isRelativeJump);
        }

        offset += ip.size;
        if (pass == 2) {
            writeInstruction(ip);
        }
    }
}

void doLabel(string label, LineTokenizer lt) {
    if (pass == 1) {
        label = stripLabel(label);
        if (symtable.find(label) != symtable.end()) {
            warning("Label redefinition");
        }

        symtable[label] = offset;
    }

    string token = lt.nextToken();
    
    if (matchesOpcode(token)) {
        doOpcode(token, lt);
    } else if (!token.empty()) {
        error("Illegal identifier");
    }
}

void assemble(string line) {
    LineTokenizer lt(line);

    string token = lt.nextToken();
    if (matchesOpcode(token)) {
        doOpcode(token, lt);
    } else if (matchesLabel(token)) {
        doLabel(token, lt);
    }

    lineNo++;
}

void dumpSymbolTable() {
    for (auto it = symtable.begin(); it != symtable.end(); it++) {
        cout << "label: " << it->first << " - address: $" << setw(4) << setfill('0') << hex << it->second << endl;
    }
}

void setOutFile(string ofn) {
    of.open(ofn, fstream::out | fstream::trunc | fstream::binary);
}

void startNextPass() {
    offset = 0;
    lineNo = 1;
    pass++;
}

void close() {
    of.flush();
    of.close();
}

bool isSuccessfulAssembly() { return success; }
void setProgramStart(uint16_t org) { 
    offset = org;

    // write the origin to the PRG file
    of.put((uint8_t) (org &~ 0xff00));
    of.put((uint8_t) (org >> 8));

    offset+=2;      // we wrote two bytes to the file for the start pointer
}