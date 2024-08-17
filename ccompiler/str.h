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
struct strStack {
    int nStrs;
    int cap;
    struct str* strs;
};


struct str StrNew();
void StrAppend(struct str* str, char c);
struct str StrFromCharArray(char* arr);
struct str StrSlice(struct str orig, int start, int len);
void StrSetLen(struct str* str, int len);
int StrGetLen(struct str str);
char StrGetChar(struct str str, int index);
char* StrGetPtr(struct str str);
struct strStack StrStackNew();
void StrStackPush(struct strStack* ss, struct str str);
struct str StrStackPeek(struct strStack* ss, int index);

#endif //STR_H
