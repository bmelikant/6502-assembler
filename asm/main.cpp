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
        "ldx #$00",
        "stx $d020",
        "stx $d021",
        "rts"
    };

    setOutFile("HELLO.PRG");
    setProgramStart(0xc000);

    for (std::string line : program) {
        assemble(line);
    }

    startNextPass();

    for (std::string line : program) {
        assemble(line);
    }

    // close the assembler context, writes file to disk
    close();
    dumpSymbolTable();

    /*
    std::cout << "--- Dumping assembled code ---" << std::endl;
    std::cout << std::hex;

    for (uint8_t byte : getObjectFile()) {
        std::cout << "$" << (int) byte << " ";
    }

    std::cout << std::endl << "--- End assembly dump ---" << std::endl;
    */
    std::cout << "Assembly was " << ((isSuccessfulAssembly()) ? "successful" : "not successful") << std::endl;
}