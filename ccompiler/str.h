#ifndef STR_H
#define STR_H
#include <stdbool.h>


//use helper functions to access
struct str {
    bool isSlice;
    int len;
    int cap;
    char* ptr;
};


//use helper functions to access
struct strList {
    int len;
    int cap;
    struct str* ptr;
};


struct str StrNew();
void StrAppend(struct str* str, char c);
struct str StrFromCharArray(char* arr);
struct str StrSlice(struct str orig, int start, int stop);
void StrSetLen(struct str* str, int len);
int StrGetLen(struct str str);
int StrGetSliceStrIndex(struct str a, struct str b);
char StrGetChar(struct str str, int index);
struct strList StrListNew();
void StrListAppend(struct strList* sl, struct str str);
bool StrListExists(struct strList* sl, struct str str);
struct str StrListGet(struct strList sl, int index);
struct str StrListGetLast(struct strList sl);
int StrListLen(struct strList sl);
bool StrEqual(struct str a, struct str b);
bool StrEqualCharArray(struct str str, char* ca);
struct str StrGetContainsBoth(struct str a, struct str b);
void StrToCharArray(struct str s, char* arrLenSPlusOne);
void StrPrint(struct str s, FILE* fp);
long long StrToLongLong(struct str s);
double StrToDouble(struct str s);
char StrToChar(struct str s);
struct str StrToString(struct str s);
struct strList StrListCopy(struct strList list);


#endif //STR_H
