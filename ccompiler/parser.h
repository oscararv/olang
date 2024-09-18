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
    BASETYPE_FUNC,
    BASETYPE_INT, //constant type
    BASETYPE_FLOAT, //constant type
    BASETYPE_CHAR, //constant type
    BASETYPE_STRING //constant type
};


struct type {
    enum baseType bType;
    struct str name;
    void* advanced; //advanced part of the type may be updated during the compilation
    bool ref; //arrays and structs can be references
    bool mut; //arrays and structs can be mutable or as function arguments
    struct token tok; //token associated with the type instance; concatenated if eligible
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


struct operandList {
    bool isSlice;
    int len;
    int cap;
    struct operand* ptr;
};



struct structTypeData {
    struct operandList members;
    struct typeList embeddedStructs;
};


union operandValue {
    long long int intVal;
    double floatVal;
    struct str stringVal;
};


struct operand {
    struct str name;
    struct type type;
    bool init;
    union operandValue value;
    struct operandList args; //only used with function operands;
    struct typeList rets; //only used with function operands;
};


struct parserContextList {
    int len;
    int cap;
    struct parserContext *ptr;
};


struct parserContext {
    struct str fileName;
    struct parserContextList imports;
    struct strList importAliases;
    struct tokenContext tc;
    struct typeList publTypes;
    struct typeList privTypes;
    struct operandList publOps;
    struct operandList privOps;
};


void ParseMainFile(char* fileName);

#endif //PARSER_H
