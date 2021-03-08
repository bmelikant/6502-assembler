#include "ltokenizer.h"

#include <algorithm>
#include <string>
#include <cctype>

bool isOtherWhitespace(unsigned char c) {
    return (isspace(c) && c != ' ');
}

std::string convertWhitespace(std::string s) {
    std::string retstr = "";
    
    // remove any odd whitespace
    for (size_t i = 0; i < s.size(); i++) {
        if (isOtherWhitespace(s[i])) retstr += " ";
        else retstr += s[i];
    }

    return retstr;
}

LineTokenizer::LineTokenizer(std::string line) {
    // format the line contents, pull out the tokens
    this->convertedString = convertWhitespace(line);
    std::string s = this->convertedString;

    while (s.length() > 0) {
        size_t firstCharLoc = s.find_first_not_of(' ');
        
        // trim whitespace
        if (firstCharLoc > 0) s.erase(0, firstCharLoc-1);
    
        // read token and store in queue
        size_t nextWhitespace = s.find_first_of(' ');
        std::string token = s.substr(0, nextWhitespace);

        this->tokenQueue.push(token);
        s.erase(0, nextWhitespace);
    }

    tokenQueue.push("");
}

std::string LineTokenizer::nextToken() {
    std::string retval = this->tokenQueue.front();
    this->tokenQueue.pop();
    return retval;
}

std::string LineTokenizer::getConvertedString() {
    return this->convertedString;
}