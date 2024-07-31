#ifndef TOKEN_H
#define TOKEN_H


enum tokenType {
    TOKEN_NEWLINE,
    TOKEN_IDENTIFIER,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHAR,
    TOKEN_IF,
    TOKEN_FOR,
    TOKEN_DEFER,
    TOKEN_SWITCH
};


struct string {
    int len;
    int cap;
    char* ptr;
};


struct stringStack {
    int nStrings;
    int cap;
    struct string* strings;
};


struct token {
    int line;
    int colStart;
    int colEnd;
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
    struct tokenPipe tokenPipe;
    struct stringStack lines;
};


#endif //TOKEN_H
