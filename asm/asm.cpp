#include "opcode.h"
#include "ltokenizer.h"

#include <iostream>

using namespace std;

/** assembler variables **/
bool success = true, allowIllegalOpcodes = false;
size_t lineNo = 0;

void error(string msg) {
    cerr << "Error (line " << lineNo << "): " << msg << endl;
    success = false;
}

void doOpcode(string mnemonic, LineTokenizer lt, int pass) {
    string token = lt.nextToken();
    string addrmode = findAddressMode(token);

    if (addrmode == "invalid") {
        error("Illegal combination of opcode and operands");
    }

    uint8_t opcode = findOpcode(mnemonic, addrmode);
    if (opcode == ILLEGAL_OPCODE) {
        error("Illegal combination of opcode and operands");
    }
}

void assemble(string line, int pass) {
    LineTokenizer lt(line);

    string token = lt.nextToken();
    if (matchesOpcode(token)) {
        doOpcode(token, lt, pass);
    }

    lineNo++;
}

