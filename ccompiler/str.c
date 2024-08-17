#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "error.h"
#include "str.h"


struct str StrNew() {
    struct str str;
    str.isSlice = false;
    str.len = 0;
    str.cap = 0;
    str.ptr = NULL;
    return str;
}


void StrAppend(struct str* str, char c) {
    if (str->isSlice) Error("string slices may not be appended");
    if (str->len >= str->cap) {
        str->cap += 100;
        str->ptr = realloc(str->ptr, sizeof(char) * str->cap);
        CheckPtr(str->ptr);
    }

    str->ptr[str->len] = c;
    str->len++;
}


struct str StrFromCharArray(char* arr) {
    struct str str = StrNew();
    for (int i = 0; i < (int)strlen(arr); i++) {
        StrAppend(&str, arr[i]);
    }
    return str;
}


struct str StrSlice(struct str orig, int start, int len) {
    if (len > orig.len) Error("string slice len is greater than original len");
    struct str str;
    str.isSlice = true;
    str.len = len - start;
    str.cap = str.len;
    str.ptr = orig.ptr + start;
    return str;
}


void StrSetLen(struct str* str, int len) {
    str->len = len;
}


int StrGetLen(struct str str) {
    return str.len;
}


char StrGetChar(struct str str, int index) {
    if (index >= str.len) Error("index out of bounds when reading string");
    return str.ptr[index];
}


char* StrGetPtr(struct str str) {
    return str.ptr;
}


struct strStack StrStackNew() {
    struct strStack ss;
    ss.nStrs = 0;
    ss.cap = 0;
    ss.strs = NULL;
    return ss;
}


void StrStackPush(struct strStack* ss, struct str str) {
    if (ss->nStrs >= ss->cap) {
        ss->cap += 100;
        ss->strs = realloc(ss->strs, sizeof(struct str));
        CheckPtr(ss->strs);
    }

    ss->strs[ss->nStrs] = str;
    ss->nStrs++;
}


struct str StrStackPeek(struct strStack* ss, int index) {
    if (index >= ss->nStrs) Error("tried to peek into string stack at invalid index");
    return ss->strs[index];
}

