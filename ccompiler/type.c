#include <stdlib.h>
#include "type.h"
#include "error.h"


void TypeListAppend(struct typeList* tl, struct type type) {
    if (tl->len >= tl->cap) {
        tl->cap += 100;
        tl->ptr = realloc(tl->ptr, sizeof(struct type) * tl->cap);
        CheckPtr(tl->ptr);
    }
    tl->ptr[tl->len] = type;
    tl->len++;
}


static struct type BaseType(char* name, enum baseType bType) {
    struct type t;
    t.bType = bType;
    t.name = StrFromCharArray(name);
    t.advanced = NULL;
    return t;
}


static void AppendBasicBaseTypes(struct typeList* tl) {
    TypeListAppend(tl, BaseType("byte", BASETYPE_BYTE));
    TypeListAppend(tl, BaseType("bool", BASETYPE_BOOL));
    TypeListAppend(tl, BaseType("int8", BASETYPE_INT8));
    TypeListAppend(tl, BaseType("int16", BASETYPE_INT16));
    TypeListAppend(tl, BaseType("int32", BASETYPE_INT32));
    TypeListAppend(tl, BaseType("int64", BASETYPE_INT64));
    TypeListAppend(tl, BaseType("uint8", BASETYPE_UINT8));
    TypeListAppend(tl, BaseType("uint16", BASETYPE_UINT16));
    TypeListAppend(tl, BaseType("uint32", BASETYPE_UINT32));
    TypeListAppend(tl, BaseType("uint64", BASETYPE_UINT64));
    TypeListAppend(tl, BaseType("float32", BASETYPE_FLOAT32));
    TypeListAppend(tl, BaseType("float64", BASETYPE_FLOAT64));
}


struct typeList TypeListNew() {
    struct typeList tl;
    tl.len = 0;
    tl.cap = 0;
    tl.ptr = NULL;
    AppendBasicBaseTypes(&tl);
    return tl;
}
