#ifndef TOKEN_H
#define TOKEN_H
#include "stdio.h"
#include "str.h"

enum tokenType {
    TOKEN_EOF,
    TOKEN_NEWLINE,
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
    TOKEN_SWITCH,
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
    TOKEN_DECREMENT
};


struct token {
    enum tokenType type;
    int lineNr;
    struct str str;
    struct tokenContext* context;
};


struct tokenPipe {
    int nAvailable;
    int nDelivered;
    int cap;
    struct token* ptr;
};


struct tokenContext {
    char* fileName;
    FILE* fp;
    struct tokenPipe tokens;
    struct strList lines;
};


struct tokenContext TokenContextNew(char* fileName);
struct token TokenPeek(struct tokenContext* tc);
struct token TokenNext(struct tokenContext* tc);
void TokenDiscardNewlines(struct tokenContext* tc);
struct token TokenNextDiscardNewlines(struct tokenContext* tc);
void TokenUnget(struct tokenContext* tc);
void TokenRestart(struct tokenContext* tc);

#endif //TOKEN_H
