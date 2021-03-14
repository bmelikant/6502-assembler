#ifndef _6502_TOKENIZER_H
#define _6502_TOKENIZER_H

#include <string>

enum TokenError {
    UnclosedLiteral
};

struct Token {
    std::string value;
    TokenError error;
};

class StringTokenizer {
public:
    StringTokenizer(std::string line);
    Token nextToken();

private:
    std::string line;
};

#endif