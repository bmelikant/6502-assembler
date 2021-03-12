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
        "test:",
        "bne begin",
        "jmp begin",
        "jmp (begin)",
        "jmp ($30ff)",
        "jmp (test)",
        "NOP",
        "NOP ; this is a test comment",
        "test2: nop",
        "test: jmp (test2)"
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
}