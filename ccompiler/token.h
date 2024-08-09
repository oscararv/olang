#ifndef TOKEN_H
#define TOKEN_H
#include "str.h"


enum tokenType {
    TOKEN_EOF,
    TOKEN_COMMENT,
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
    TOKEN_BOOL,
    TOKEN_BYTE,
    TOKEN_INT8,
    TOKEN_INT16,
    TOKEN_INT32,
    TOKEN_INT64,
    TOKEN_UINT8,
    TOKEN_UINT16,
    TOKEN_UINT32,
    TOKEN_UINT64,
    TOKEN_FLOAT32,
    TOKEN_FLOAT64,
    TOKEN_STRUCT,
    TOKEN_VOCAB,
    TOKEN_FUNC
};


struct token {
    enum tokenType type;
    int lineNr;
    struct string str;
    struct tokenContext* context;
};


struct tokenPipe {
    int nAvailable;
    int nDelivered;
    int cap;
    struct token* tokens;
};


struct tokenContext {
    char* fileName;
    FILE* fp;
    struct tokenPipe tokens;
    struct stringStack lines;
};


struct tokenContext TokenContextNew(char* fileName);
struct token TokenNext(struct tokenContext* tc);
struct token TokenNextDiscardNewlines(struct tokenContext* tc);


#endif //TOKEN_H
