#include "opcode.h"
#include "ltokenizer.h"

using namespace std;

void doOpcode(string opcode, LineTokenizer lt, int pass) {
    string token = lt.nextToken();
    
}

void assemble(string line, int pass) {
    LineTokenizer lt(line);

    string token = lt.nextToken();
    if (matchesOpcode(token)) {
        doOpcode(token, lt, pass);
    }
}

