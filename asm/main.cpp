#include "assembler.h"
#include "ltokenizer.h"

#include <iostream>

int main(int argc, char *argv[]) {
    std::string ts = "This      is\ta\ttest\vstring";
    std::cout << ts << std::endl;

    LineTokenizer lt(ts);

    std::cout << lt.getConvertedString() << std::endl;
}