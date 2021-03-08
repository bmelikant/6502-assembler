#ifndef LINE_TOKENIZER_6502_H
#define LINE_TOKENIZER_6502_H

#include <string>
#include <queue>

class LineTokenizer {
public:

    LineTokenizer(std::string);
    std::string nextToken();

private:

    std::queue<std::string> tokenQueue;
};

#endif