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
    int nStrs;
    int cap;
    struct str* strs;
};


//assumes a contains b
int StrGetSubStrIndex(struct str a, struct str b);

struct str StrNew();
void StrAppend(struct str* str, char c);
struct str StrFromCharArray(char* arr);
struct str StrSlice(struct str orig, int start, int len);
void StrSetLen(struct str* str, int len);
int StrGetLen(struct str str);
char StrGetChar(struct str str, int index);
struct strList StrListNew();
void StrListAppend(struct strList* ss, struct str str);
bool StrListExists(struct strList* ss, struct str str);
struct str StrListGet(struct strList* ss, int index);
bool StrEqual(struct str a, struct str b);
bool StrEqualCharArray(struct str str, char* ca);

#endif //STR_H
