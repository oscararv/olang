#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "token.h"
#include "error.h"


void ParseTypedef(struct parserContext* pc) {
    (void)pc;
}


void ParseGlobalLevel(struct parserContext* pc) {
    struct token tok = TokenNextDiscardNewlines(&(pc->tc));
    switch (tok.type) {
        case TOKEN_TYPE: ParseTypedef(pc); break;
        default: SyntaxErrorInvalidToken(tok, NULL); 
    }
}


struct parserContext parserContextNew(char* fileName) {
    struct parserContext pc;
    pc.tc = TokenContextNew(fileName);
    return pc;
}


void ParseFile(char* fileName) {
    struct parserContext pc = parserContextNew(fileName);
    struct token tok;
    while ((tok = TokenNextDiscardNewlines(&(pc.tc))).type != TOKEN_EOF) {
        TokenUnget(&(pc.tc), tok);
        ParseGlobalLevel(&pc);
    }
}
