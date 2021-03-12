#include "opcode.h"

#include <regex>
#include <map>
#include <cstring>
#include <cctype>

#include <iostream>

using namespace std;

// matrix and frequent hit cache
extern const Opcode opcodeMatrix[16][16];
extern const string mnemonics[];

struct AddressMode {
    string regex;
    int bytes;
};

// addressing modes and their regex patterns
static const map<string, AddressMode> addressModes = {
    { "", { "", 0 } },
    { "imm", { "^#\\$([0-9a-f]{1,2})$", 2 } },
    { "zp",  { "^\\$([0-9a-f]{1,2})$", 2 } },
    { "zpx", { "^\\$([0-9a-f]{1,2}),x$", 2 } },
    { "abs", { "^\\$([0-9a-f]{3,4})$", 3 } },
    { "abx", { "^\\$([0-9a-f]{3,4}),x$", 3 } },
    { "aby", { "^\\$([0-9a-f]{3,4}),y$", 3 } },
    { "izx", { "^\\(\\$([0-9a-f]{1,2}),x\\)$", 2 } },
    { "izy", { "^\\(\\$([0-9a-f]{1,2})\\),y$", 2 } },
    { "ind", { "^\\(\\$([0-9a-f]{3,4})\\)$", 3 } },
    { "rel", { "^\\(\\$([0-9a-f]{1,2})\\)$", 2 } },
    { "label-abs", { "^[0-9a-zA-z_]*$", 3 } },
    { "label-rel", { "^\\(([0-9a-zA-z_]*)\\)$", 2 } }
};

string tolower(string s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (isupper(s[i])) s[i] = tolower(s[i]);
    }
    return s;
}

string toupper(string s) {
    for (size_t i = 0; i < s.length(); i++) {
        if (islower(s[i])) s[i] = toupper(s[i]);
    }
    return s;
}

uint8_t findOpcode(string mnemonic, string addrmode) {
    if (addrmode.find("label-") != string::npos) {
        addrmode = addrmode.erase(0, addrmode.find("-")+1);
    }
    cout << "ADDR MODE IS " << addrmode << endl;
    // create an opcode variable
    Opcode o = { .mnemonic = mnemonic, .addrmode = addrmode };
    return findOpcode(o);
}

uint8_t findOpcode(Opcode o) {
    cout << "opcode (" << o.mnemonic << ") addrmode (" << o.addrmode << ")" << endl;
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
    map<string,AddressMode>::const_iterator it = addressModes.begin();
    while (it != addressModes.end()) {
        if (it->second.regex.empty() && argument.empty()) {
            return "";
        }

        regex rgx = regex(it->second.regex);
        if (regex_match(argument, rgx) == true) return it->first;
        it++;
    }

    return "invalid";
}

string stripLabel(string label, string addrmode) {
    if (addrmode == "label-abs") return label;
    regex rgx = regex(addressModes.find(addrmode)->second.regex);
    smatch match;

    if (regex_search(label, match, rgx) == true) {
        return match.str(1);
    }

    return label;
}

int getInstructionSize(string addressMode) {
    return addressModes.find(addressMode)->second.bytes;
}

bool matchesOpcode(string token) {
    const string *search = mnemonics;
    while (!search->empty()) {
        if (*search == toupper(token)) return true;
        ++search;
    }

    return false;
}

bool argumentIsLabelType(string token) {
    return (token == "label-rel" || token == "label-abs");
}

bool Opcode::operator==(const Opcode& opcode) {
    return (tolower(mnemonic) == tolower(opcode.mnemonic) && tolower(addrmode) == tolower(opcode.addrmode));
}