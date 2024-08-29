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


//assumes a contains b
int StrGetSubStrIndex(struct str a, struct str b) {
    return b.ptr - a.ptr;
}


char StrGetChar(struct str str, int index) {
    if (index >= str.len) Error("index out of bounds when reading string");
    return str.ptr[index];
}


struct strList StrListNew() {
    struct strList ss;
    ss.nStrs = 0;
    ss.cap = 0;
    ss.strs = NULL;
    return ss;
}


void StrListAppend(struct strList* ss, struct str str) {
    if (ss->nStrs >= ss->cap) {
        ss->cap += 100;
        ss->strs = realloc(ss->strs, sizeof(struct str));
        CheckPtr(ss->strs);
    }

    ss->strs[ss->nStrs] = str;
    ss->nStrs++;
}


bool StrListExists(struct strList* ss, struct str str) {
    for (int i = 0; i < ss->nStrs; i++) if (StrEqual(ss->strs[i], str)) return true;
    return false;
}


struct str StrListGet(struct strList* ss, int index) {
    if (index >= ss->nStrs) Error("invalid string list index");
    return ss->strs[index];
}


bool StrEqual(struct str a, struct str b) {
    if (a.len != b.len) return false;
    for (int i = 0; i < a.len; i++) {
        if (a.ptr[i] != b.ptr[i]) return false;
    }
    return true;
}


bool StrEqualCharArray(struct str str, char* ca) {
    int caLen = strlen(ca);
    if (str.len != caLen) return false;
    for (int i = 0; i < str.len; i++) {
        if (str.ptr[i] != ca[i]) return false;
    }
    return true;
}
