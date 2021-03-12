//#include "assembler.h"
#include "asm.h"
#include "opcode.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

int main(int argc, char *argv[]) { 
    
    std::vector<std::string> program = {
        "begin:",
        "ADC #$01",
        "ADC $10fb",
        "adc ($44,x)",
        "bne begin",
        "jmp begin",
        "jmp (begin)",
        "jmp ($30ff)",
        "jmp (test)",
        "NOP",
        "NOP ; this is a test comment"
    };

    // insert a test symbol
    //insertSymbol("begin", 0x0002);
    //insertSymbol("test",0x30ff);

    for (std::string line : program) {
        assemble(line, 1);
    }

    dumpSymbolTable();

    /*
    prepareForSecondPass();

    for (std::string line : program) {
        assemble(line, 2);
    }

    std::cout << "--- Dumping assembled code ---" << std::endl;
    std::cout << std::hex;

    for (uint8_t byte : getObjectFile()) {
        std::cout << "$" << (int) byte << " ";
    }

    std::cout << std::endl << "--- End assembly dump ---" << std::endl;
    std::cout << "Assembly was " << ((isSuccessfulAssembly()) ? "successful" : "not successful") << std::endl;
    */

    Opcode opcodes[] = {
        { .mnemonic = "BRK", .addrmode = "" },
        { .mnemonic = "ORA", .addrmode = "izy" },
        { .mnemonic = "CLI", .addrmode = "" },
        { .mnemonic = "INC", .addrmode = "zpx" },
        { .mnemonic = "CLI", .addrmode = "zpx" },
        { .mnemonic = "ISC", .addrmode = "abx" }
    };

    for (size_t i = 0; i < 6; i++) {
        uint8_t opcode = findOpcode(opcodes[i]);
        if (opcode == ILLEGAL_OPCODE) {
            std::cout << "Illegal opcode detected: " << opcodes[i].mnemonic << "," << opcodes[i].addrmode << std::endl;
        } else {
            std::cout << "Mnemonic: " << opcodes[i].mnemonic << ", addrmode: " << opcodes[i].addrmode << ", opcode: " << std::hex << (int) opcode << std::endl;
        }
    }

    std::cout << "The address mode for $3f2c is " << findAddressMode("$3f2c") << std::endl;
    std::cout << "The address mode for ($3f2c) is " << findAddressMode("($3f2c)") << std::endl;
    std::cout << "The address mode for ($3f),x is " << findAddressMode("($3f),x") << std::endl;
    std::cout << "The address mode for $3f,x is " << findAddressMode("$3f,x") << std::endl;
}