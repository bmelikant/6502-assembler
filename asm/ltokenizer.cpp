#include "ltokenizer.h"

#include <sstream>
#include <algorithm>
#include <string>
#include <cctype>

/**
 * returns true if the character is whitespace, but does NOT return true for the space character
 */
bool isOtherWhitespace(unsigned char c) {
    return (isspace(c) && c != ' ');
}

/**
 * anything that is not a space that is in the string... turn it into a space
 */
std::string convertWhitespace(std::string s) {
    std::string retstr = "";
    
    // remove any odd whitespace
    for (size_t i = 0; i < s.size(); i++) {
        if (isOtherWhitespace(s[i])) retstr += " ";
        else retstr += s[i];
    }

    return retstr;
}

extern std::string tolower(std::string inp);

// modify later to handle single- and double-quoted strings
bool canLowercaseToken(std::string token) {
    return true;
}

LineTokenizer::LineTokenizer(std::string line) {
    // format the line contents, pull out the tokens
    this->convertedString = convertWhitespace(line);
    std::string s = this->convertedString;

    std::stringstream ss(s);
    std::string token;

    while (ss.good()) {
        ss >> token;
        if (token[0] == ';') break;

        if (canLowercaseToken(token)) {
            token = tolower(token);
        }
        
        tokenQueue.push(token);
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