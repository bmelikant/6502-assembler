#include "opcode.h"

#include <regex>
#include <map>
#include <cstring>

using namespace std;

// matrix and frequent hit cache
extern const Opcode opcodeMatrix[16][16];
extern const string mnemonics[];

/*
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
*/

// addressing modes and their regex patterns
static const map<string, string> addressModes = {
    { "", "" },
    { "imm", "^#\\$([0-9a-f]{1,2})$" },
    { "zp", "^\\$([0-9a-f]{1,2})$" },
    { "zpx", "^\\$([0-9a-f]{1,2}),x$" },
    { "abs", "^\\$([0-9a-f]{3,4})$" },
    { "abx", "^\\$([0-9a-f]{3,4}),x$" },
    { "aby", "^\\$([0-9a-f]{3,4}),y$" },
    { "izx", "^\\(\\$([0-9a-f]{1,2}),x\\)$" },
    { "izy", "^\\(\\$([0-9a-f]{1,2})\\),y$" },
    { "ind", "^\\(\\$([0-9a-f]{3,4})\\)$" },
    { "rel", "^\\(\\$([0-9a-f]{1,2})\\)$" },
    { "label-abs", "^[0-9a-zA-z_]*$" },
    { "label-rel", "^\\(([0-9a-zA-z_]*)\\)$" }
};

uint8_t findOpcode(string mnemonic, string addrmode) {
    // create an opcode variable
    Opcode o = { .mnemonic = mnemonic, .addrmode = addrmode };
    return findOpcode(o);
}

uint8_t findOpcode(Opcode o) {
    // search each matrix row
    for (size_t i = 0; i < 16; i++) {
        // search each matrix column
        for (size_t j = 0; j < 16; j++) {
            if (o == opcodeMatrix[i][j]) {
                return (i*16)+j;        // the 8 bit opcode is formed by the location in the matrix
            }
        }
    }

    return ILLEGAL_OPCODE;
}

string findAddressMode(string argument) {
    std::map<string,string>::const_iterator it = addressModes.begin();
    while (it != addressModes.end()) {
        if (it->second.empty() && argument.empty()) {
            return "";
        }

        regex rgx = regex(it->second);
        if (regex_match(argument, rgx) == true) return it->first;
        it++;
    }

    return "invalid";
}

bool matchesOpcode(std::string token) {
    const string *search = mnemonics;
    while (!search->empty()) {
        if (*search == token) return true;
        ++search;
    }
}

bool Opcode::operator==(const Opcode& opcode) {
    return (mnemonic == opcode.mnemonic && addrmode == opcode.addrmode);
}