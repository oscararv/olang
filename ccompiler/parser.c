#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "token.h"


struct parserContext {
    struct tokenContext tc;
};


void ParseGlobalLevel(struct parserContext* pc) {
    struct token tok;
    while((tok = TokenNextDiscardNewlines(&(pc->tc))).type != TOKEN_EOF);
}


struct parserContext parserContextNew(char* fileName) {
    struct parserContext pc;
    pc.tc = TokenContextNew(fileName);
    return pc;
}


void ParseFile(char* fileName) {
    struct parserContext pc = parserContextNew(fileName);
    ParseGlobalLevel(&pc);
}
