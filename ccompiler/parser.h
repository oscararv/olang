#ifndef PARSER_H
#define PARSER_H
#include "token.h"


enum baseType {
    BASETYPE_BYTE,
    BASETYPE_BOOL,
    BASETYPE_INT8,
    BASETYPE_INT16,
    BASETYPE_INT32,
    BASETYPE_INT64,
    BASETYPE_UINT8,
    BASETYPE_UINT16,
    BASETYPE_UINT32,
    BASETYPE_UINT64,
    BASETYPE_FLOAT32,
    BASETYPE_FLOAT64,
    BASETYPE_ARRAY,
    BASETYPE_STRUCT,
    BASETYPE_VOCAB,
    BASETYPE_FUNC
};

struct type {
    enum baseType bType;
    struct str name;
    void* advanced;
};


struct arrTypeData {
    struct type heldType;
    int len;
};


struct vocabTypeData {
    struct strList words;
};


struct typeList {
    int len;
    int cap;
    struct type* ptr;
};


struct funcTypeData {
    struct typeList args;
    struct typeList rets;
};


struct operand {
    struct str name;
    struct type type;
    char* value;
};


struct operandList {
    int len;
    int cap;
    struct operand* ptr;
};


struct structTypeData {
    struct operandList members;
};


struct parserContext {
    struct tokenContext tc;
    struct typeList publTypes;
    struct typeList privTypes;
};

void ParseFile(char* fileName);

#endif //PARSER_H
