#ifndef TYPE_H
#define TYPE_H
#include "str.h"

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

struct typeList {
    int len;
    int cap;
    struct type* ptr;
};

#endif //TYPE_H
