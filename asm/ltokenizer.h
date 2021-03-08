#ifndef LINE_TOKENIZER_6502_H
#define LINE_TOKENIZER_6502_H

#include <string>
#include <queue>

class LineTokenizer {
public:

    LineTokenizer(std::string);
    std::string nextToken();
    std::string getConvertedString();

private:

    std::queue<std::string> tokenQueue;
    std::string convertedString;
};

#endif