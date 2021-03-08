#include "assembler.h"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[]) {
    std::vector<std::string> program = {
        "ADC #$01",
        "ADC $10fb",
        "adc ($44,x)",
        "NOP",
        "NOP ; this is a test comment"
    };

    for (std::string line : program) {
        assemble(line, 1);
    }
}