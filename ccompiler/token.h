#ifndef TOKEN_H
#define TOKEN_H
#include "stdio.h"
#include "str.h"


enum tokenType {
    TOKEN_UNDEF = 0,
    TOKEN_EOF,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_STRING,
    TOKEN_IDENTIFIER,
    TOKEN_IMPORT,
    TOKEN_TYPE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_DEFER,
    TOKEN_RETURN,
    TOKEN_BREAK,
    TOKEN_MATCH,
    TOKEN_CASE,
    TOKEN_STRUCT,
    TOKEN_VOCAB,
    TOKEN_FUNC,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_PAREN_OPEN,
    TOKEN_PAREN_CLOSE,
    TOKEN_SQUAREBRACKET_OPEN,
    TOKEN_SQUAREBRACKET_CLOSE,
    TOKEN_CURLYBRACKET_OPEN,
    TOKEN_CURLYBRACKET_CLOSE,
    TOKEN_ASSIGNMENT,
    TOKEN_ASSIGNMENT_ADD,
    TOKEN_ASSIGNMENT_SUB,
    TOKEN_ASSIGNMENT_MUL,
    TOKEN_ASSIGNMENT_DIV,
    TOKEN_ASSIGNMENT_MODULO,
    TOKEN_LOGICAL_EQUALS,
    TOKEN_LOGICAL_NOT_EQUALS,
    TOKEN_LOGICAL_NOT,
    TOKEN_LOGICAL_AND,
    TOKEN_LOGICAL_OR,
    TOKEN_LOGICAL_LESS_THAN,
    TOKEN_LOGICAL_LESS_THAN_OR_EQUAL,
    TOKEN_LOGICAL_GREATER_THAN,
    TOKEN_LOGICAL_GREATER_THAN_OR_EQUAL,
    TOKEN_BITWISE_AND,
    TOKEN_BITWISE_OR,
    TOKEN_BITWISE_XOR,
    TOKEN_BITWISE_COMPLEMENT,
    TOKEN_BITSHIFT_LEFT,
    TOKEN_BITSHIFT_RIGHT,
    TOKEN_ADD,
    TOKEN_SUB,
    TOKEN_MUL,
    TOKEN_DIV,
    TOKEN_MODULO,
    TOKEN_INCREMENT,
    TOKEN_DECREMENT,
    TOKEN_MUT
};


struct token {
    enum tokenType type;
    int lineNr;
    struct str str;
    struct tokenContext* context;
};


struct tokenList {
    int len;
    int cap;
    bool isSlice;
    struct token* ptr;
};


struct tokenPipe {
    int nAvailable;
    int nDelivered;
    int cap;
    struct token* ptr;
};


struct tokenContext {
    struct str fileName;
    FILE* fp;
    struct tokenPipe tokens;
    struct strList lines;
};


#define LINE_NR_UNDEFINED -1
static const struct token TOKEN_UNDEFINED = {.type = TOKEN_UNDEF, .lineNr = LINE_NR_UNDEFINED};


struct tokenList TokenListNew();
void TokenListAppend(struct tokenList* tl, struct token tok);
struct tokenList TokenListSlice(struct tokenList tl, int start, int stop);
struct tokenContext* TokenContextNew(struct str fileName);
struct token TokenNext(struct tokenContext* tc);
struct token TokenPeek(struct tokenContext* tc);
void TokenUnget(struct tokenContext* tc, int n);
void TokenRestart(struct tokenContext* tc);
struct token TokenExtend(struct token base, struct token tail); //assumes the tokens exist on the same line
char* TokenTypeToString(enum tokenType t);


#endif //TOKEN_H
