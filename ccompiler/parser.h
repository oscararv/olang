#ifndef PARSER_H
#define PARSER_H
#include "token.h"


#define ARRAY_REF -1 //must not interfere with a valid array length


enum baseType {
    BASETYPE_PLACEHOLDER, //type definition type after first pass
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
    BASETYPE_IMPORT,
    BASETYPE_INT, //constant type
    BASETYPE_FLOAT, //constant type
    BASETYPE_CHAR, //constant type
    BASETYPE_STRING //constant type
};


enum operationType {
    OPERATION_NOOP,
    OPERATION_TYPECAST,
    OPERATION_FUNC,
    OPERATION_MUL,
    OPERATION_DIV,
    OPERATION_ADD,
    OPERATION_SUB,
    OPERATION_MODULO,
    OPERATION_INCREMENT,
    OPERATION_DECREMENT,
    OPERATION_LOGICAL_EQUALS,
    OPERATION_LOGICAL_NOT,
    OPERATION_LOGICAL_AND,
    OPERATION_LOGICAL_OR,
    OPERATION_LOGICAL_LESS_THAN,
    OPERATION_LOGICAL_LESS_THAN_OR_EQUAL,
    OPERATION_LOGICAL_GREATER_THAN,
    OPERATION_LOGICAL_GREATER_THAN_OR_EQUAL,
    OPERATION_BITWISE_AND,
    OPERATION_BITWISE_OR,
    OPERATION_BITWISE_XOR,
    OPERATION_BITWISE_COMPLEMENT,
    OPERATION_BITSHIFT_LEFT,
    OPERATION_BITSHIFT_RIGHT
};


struct opPtrList {
    int len;
    int cap;
    struct operand** ptr;
};


struct varList {
    int len;
    int cap;
    struct variable* ptr;
};


struct typeList {
    int len;
    int cap;
    struct type* ptr;
};


struct typePtrList {
    int len;
    int cap;
    struct type** ptr;
};


//type definitions are not themselves usable types
//rather types reference a type definition
//type definitions may only be allocated once
//they are unnamed and allocated ad hoc
//may be updated during the compilation
struct typeDef {
    bool complete;
    struct strList words;
    struct varList args;
    struct typeList rets;
    struct varList members;
    struct typeList embeddedStructs;
};


//types must reference a type definition
//there may be several type instances per type definition
struct type {
    struct str name;
    struct token tok;
    enum baseType bType;
    struct typeDef* def;
    bool ref; //arrays are refs if dimension one is ARRAY_REF; struct are refs unless they are declared "{}"
    bool mut;
    struct opPtrList arrLenghts;
    enum baseType arrBType;
    struct typePtrList dependants; //types aliasing this type before its base type is defined
};


//operands are unnamed and always allocated ad hoc
struct operand {
    struct token tok;
    struct type type;
    bool valKnown;
    long long intVal;
    double floatVal;
    struct str strVal;
    struct opPtrList operands;
    enum operationType operation;
};


//variables may ever only be part of one variable list
//variables can be unnamed and allocated in local variable lists
//variables allocated in parser context lists must be named
struct variable {
    struct str name;
    struct token tok;
    struct type type;
    struct operand* value;
};


struct pcList {
    int len;
    int cap;
    struct parserContext* ptr;
};


struct parserContext {
    struct str fileName;
    struct tokenContext* tc;
    struct pcList* parsedFiles;
    struct typeList publTypes;
    struct typeList privTypes;
    struct varList publVarsAndConsts;
    struct varList privVarsAndConsts;
};


void ParseMainFile(char* fileName);


#endif //PARSER_H
