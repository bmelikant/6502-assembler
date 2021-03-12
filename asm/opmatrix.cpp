#include "opcode.h"

#include <regex>
#include <map>
#include <cstring>
#include <cctype>

#include <iostream>

using namespace std;

// matrix and frequent hit cache
extern const Mnemonic opcodeMatrix[16][16];
extern const string mnemonics[];

struct AddressMode {
    string regex;
    int bytes;
};

Mnemonic IllegalMnemonic = { .mnemonic = "", .addrmode = "" };
InstructionPacket IllegalInstruction = { .opcode = ILLEGAL_OPCODE, .argument = 0, .argSize = 0, .label = "", .isLabelType = false };

// addressing modes and their regex patterns
static const map<string, AddressMode> addressModes = {
    { "", { "$(?!\\s\\S)$", 1 } },
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
    { "label-abs", { "^[0-9a-zA-Z_]*$", 3 } },
    { "label-ind", { "^\\(([0-9a-zA-Z_]*)\\)$"} },
    { "label-rel", { "^[0-9a-zA-Z_]*$", 2 } }
};

string tolower(string s) {
    string d = s;
    for (size_t i = 0; i < d.length(); i++) {
        if (isupper(d[i])) d[i] = tolower(d[i]);
    }
    return d;
}

string toupper(string s) {
    string d = s;
    for (size_t i = 0; i < d.length(); i++) {
        if (islower(d[i])) d[i] = toupper(d[i]);
    }
    return d;
}

string stripLabel(string label, string addrmode) {
    if (addrmode == "label-abs" || addrmode == "label-rel") return label;
    regex rgx = regex(addressModes.find(addrmode)->second.regex);
    smatch match;

    if (regex_search(label, match, rgx) == true) {
        return match.str(1);
    }

    return label;
}

bool matchesOpcode(string token) {
    const string *search = mnemonics;
    while (!search->empty()) {
        if (*search == toupper(token)) return true;
        ++search;
    }

    return false;
}

bool addressModeIsLabelType(string token) {
    return (token.find("label-") == 0);
}

uint8_t findOpcodeAddress(string mnemonic, string addrmode) {
    Mnemonic m = { .mnemonic = toupper(mnemonic), .addrmode = addrmode };
    // search each matrix row
    for (size_t i = 0; i < 16; i++) {
        // search each matrix column
        for (size_t j = 0; j < 16; j++) {
            if (m == opcodeMatrix[i][j]) {
                return (i*16)+j;        // the 8 bit opcode is formed by the location in the matrix
            }
        }
    }

    return ILLEGAL_OPCODE;
}

InstructionPacket createInstructionPacket(uint8_t opcode, string argument, string addrmode) {
    AddressMode mode = addressModes.find(addrmode)->second;
    InstructionPacket ip;
    ip.opcode = opcode;
    ip.argSize = mode.bytes;
    ip.argument = 0;

    if (addressModeIsLabelType(addrmode)) {
        ip.isLabelType = true;
        ip.label = stripLabel(argument, addrmode);
    } else if (!addrmode.empty()) {
        smatch match;
        if (regex_search(argument, match, regex(mode.regex)) == true) {
            ip.argument = stoi(match.str(1), 0, 16);
        }
    }

    return ip;
}

InstructionPacket buildInstruction(string mnemonic, string argument) {
    map<string,AddressMode>::const_iterator it = addressModes.begin();
    while (it != addressModes.end()) {
        regex rgx = regex(it->second.regex);
        if (regex_match(argument, rgx) == true) {
            string addrmode = it->first;

            string real_addrmode = addrmode;
            if (addressModeIsLabelType(addrmode)) real_addrmode = addrmode.substr(addrmode.find("-")+1, addrmode.length()-1);

            // passing real_addrmode accounts for label type arguments that are technically rel or abs
            uint8_t opcode = findOpcodeAddress(mnemonic, real_addrmode);
            if (opcode != ILLEGAL_OPCODE) {
                return createInstructionPacket(opcode, argument, addrmode);       
            }
        }
        it++;
    }

    return IllegalInstruction;
}

bool Mnemonic::operator==(const Mnemonic& opcode) {
    return (tolower(mnemonic) == tolower(opcode.mnemonic) && tolower(addrmode) == tolower(opcode.addrmode));
}

bool InstructionPacket::operator==(const InstructionPacket& packet) {
    return (opcode == packet.opcode && argument == packet.argument &&
        argSize == packet.argSize && label == packet.label && isLabelType == packet.isLabelType);
}