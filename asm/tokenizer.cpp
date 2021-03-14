#include "tokenizer.h"

#include <string>
#include <cctype>

using namespace std;

StringTokenizer::StringTokenizer(string line) {
    this->line = line;
}

string StringTokenizer::nextToken() {
    string token = "";
    size_t i = 0;

    // skip leading whitespace
    while (isspace(line[i++])) ;

    // if single or double quote, this is a quoted string. read until end quote
    
}