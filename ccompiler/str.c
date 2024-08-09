#include <stdlib.h>
#include <stdio.h>
#include "error.h"
#include "str.h"


struct string StringNew() {
    struct string str;
    str.len = 0;
    str.cap = 0;
    str.ptr = NULL;
    return str;
}


struct string StringSlice(struct string* orig, int start, int len) {
    if (len > orig->len) Error("string slice len is longer than original len");
    struct string str;
    str.len = len - start;
    str.cap = orig->cap - start;
    str.ptr = orig->ptr + start;
    return str;
}


void StringSetLen(struct string* str, int len) {
    str->len = len;
}


void StringAppend(struct string* str, char c) {
    if (str->len >= str->cap) {
        str->cap += 100;
        str->ptr = realloc(str->ptr, sizeof(char) * str->cap);
        CheckPtr(str->ptr);
    }

    str->ptr[str->len] = c;
    str->len++;
}


char StringGetLen(struct string str) {
    return str.len;
}


char StringGet(struct string str, int index) {
    if (index >= str.len) Error("index out of bounds when reading string");
    return str.ptr[index];
}


char* StringGetPtr(struct string str) {
    return str.ptr;
}


struct stringStack StringStackNew() {
    struct stringStack ss;
    ss.nStrings = 0;
    ss.cap = 0;
    ss.strings = NULL;
    return ss;
}


void StringStackPush(struct stringStack* ss, struct string str) {
    if (ss->nStrings >= ss->cap) {
        ss->cap += 100;
        ss->strings = realloc(ss->strings, sizeof(struct string));
        CheckPtr(ss->strings);
    }

    ss->strings[ss->nStrings] = str;
    ss->nStrings++;
}


struct string StringStackPeek(struct stringStack* ss, int index) {
    if (index >= ss->nStrings) Error("tried to peek into string stack at invalid index");
    return ss->strings[index];
}

