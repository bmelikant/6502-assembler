#ifndef _6502_TOKENIZER_H
#define _6502_TOKENIZER_H

#include <string>

class StringTokenizer {
public:
    StringTokenizer(std::string line);
    std::string nextToken();

private:
    std::string line;
    
};

#endif