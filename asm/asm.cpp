#include "opcode.h"
#include "ltokenizer.h"

#include <iostream>
#include <iomanip>
#include <map>
#include <regex>

using namespace std;

/** assembler variables **/
uint16_t origin = 0, offset = 0;
bool success = true, allowIllegalOpcodes = false;
size_t lineNo = 1;
map<string,uint16_t> symtable;

void error(string msg) {
    cerr << "Error (line " << dec << lineNo << "): " << msg << endl;
    success = false;
}

void warning(string msg) {
    cout << "Warning (line " << dec << lineNo << "): " << msg << endl;
}

void printInstruction(uint8_t opcode, uint16_t operand, string label) {
    cout << "Opcode: $" << hex << setw(2) << (int) opcode << ", operand: $" << setw(4) << setfill('0') << operand << ", label: " << label << endl;
}

const char *labelRegex = "^([0-9a-zA-Z_]*):?$";
bool matchesLabel(string token) {
    return (regex_match(token, regex(labelRegex)));
}

string stripLabel(string label) {
    regex re = regex(labelRegex);
    smatch match;

    if (regex_search(label, match, re) == true) {
        return match.str(1);
    }

    return label;
}

void doOpcode(string mnemonic, LineTokenizer lt, int pass) {
    string token = lt.nextToken();

    InstructionPacket ip = buildInstruction(mnemonic, token);
    if (ip == IllegalInstruction) {
        error("Illegal combination of opcode and operands");
    } else {
        if (pass == 2 && ip.isLabelType) {
            // on the second pass, grab label addresses
        }

        offset += ip.argSize;
        printInstruction(ip.opcode, ip.argument, ip.label);
    }
}

void doLabel(string label, LineTokenizer lt, int pass) {
    if (pass == 1) {
        label = stripLabel(label);
        if (symtable.find(label) != symtable.end()) {
            warning("Label redefinition");
        }

        symtable[label] = offset;
    }

    string token = lt.nextToken();
    
    if (matchesOpcode(token)) {
        doOpcode(token, lt, pass);
    } else if (!token.empty()) {
        error("Illegal identifier");
    }
}

void assemble(string line, int pass) {
    LineTokenizer lt(line);

    string token = lt.nextToken();
    if (matchesOpcode(token)) {
        doOpcode(token, lt, pass);
    } else if (matchesLabel(token)) {
        doLabel(token, lt, pass);
    }

    lineNo++;
}

void dumpSymbolTable() {
    for (auto it = symtable.begin(); it != symtable.end(); it++) {
        cout << "label: " << it->first << " - address: $" << setw(4) << setfill('0') << hex << it->second << endl;
    }
}
