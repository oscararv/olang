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
        str->ptr = realloc(str->ptr, sizeof(*(str->ptr)) * str->cap);
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


struct str StrSlice(struct str orig, int start, int stop) {
    if (stop > orig.len) Error("string slice stop index is greater than original string len");
    struct str str;
    str.isSlice = true;
    str.len = stop - start;
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


int StrGetSliceStrIndex(struct str a, struct str b) {
    int diff = b.ptr - a.ptr;
    if (diff < 0 || diff >= a.len) Error("not a valid string slice");
    return b.ptr - a.ptr;
}


char StrGetChar(struct str str, int index) {
    if (index < 0 || index >= str.len) Error("index out of bounds when reading string");
    return str.ptr[index];
}


struct strList StrListNew() {
    struct strList sl;
    sl.len = 0;
    sl.cap = 0;
    sl.ptr = NULL;
    return sl;
}


void StrListAppend(struct strList* sl, struct str str) {
    if (sl->len >= sl->cap) {
        sl->cap += 100;
        sl->ptr = realloc(sl->ptr, sizeof(*(sl->ptr)) * sl->cap);
        CheckPtr(sl->ptr);
    }

    sl->ptr[sl->len] = str;
    sl->len++;
}


bool StrListExists(struct strList* sl, struct str str) {
    for (int i = 0; i < sl->len; i++) if (StrEqual(sl->ptr[i], str)) return true;
    return false;
}


struct str StrListGet(struct strList sl, int index) {
    if (index < 0 || index >= sl.len) Error("invalid string list index");
    return sl.ptr[index];
}


struct str StrListGetLast(struct strList sl) {
    if (sl.len < 1) Error("no strings added to string list yet");
    return sl.ptr[sl.len -1];
}


int StrListLen(struct strList sl) {
    return sl.len;
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


//assumes a is followed by b in memory
struct str StrGetContainsBoth(struct str a, struct str b) {
    struct str ret;
    ret.isSlice = true;
    ret.len = b.ptr - a.ptr + b.len;
    ret.cap = ret.len;
    ret.ptr = a.ptr;
    return ret;
}


void StrToCharArray(struct str s, char* arrLenSPlusOne) {
    arrLenSPlusOne[s.len] = '\0';
    for (int i = 0; i < s.len; i++) {
        arrLenSPlusOne[i] = s.ptr[i];
    }
}


void StrPrint(struct str s, FILE* fp) {
    char printArr[s.len +1];
    StrToCharArray(s, printArr);
    fputs(printArr, fp);
}


long long StrToLongLongVal(struct str s) {
    char arr[StrGetLen(s) +1];
    strncpy(arr, s.ptr, StrGetLen(s));
    arr[StrGetLen(s)] = '\0';
    return atoll(arr);
}


double StrToDoubleVal(struct str s) {
    char arr[StrGetLen(s) +1];
    strncpy(arr, s.ptr, StrGetLen(s));
    arr[StrGetLen(s)] = '\0';
    return atof(arr);
}


char StrToCharVal(struct str s) {
    if (StrGetChar(s, 1) != '\\') return StrGetChar(s, 1);
    else if (StrGetChar(s, 2) == 'n') return '\n';
    else if (StrGetChar(s, 2) == 't') return '\t';
    else if (StrGetChar(s, 2) == '\\') return '\\';
    else if (StrGetChar(s, 2) == '\'') return '\'';
    else exit(EXIT_FAILURE); //unreachable
}


struct str StrToStringVal(struct str s) {
    return StrSlice(s, 1, StrGetLen(s) -1);
}
