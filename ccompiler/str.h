#ifndef STR_H
#define STR_H


//fields should NOT be accessed directly
struct string {
    int len;
    int cap;
    char* ptr;
};


//fields should NOT be accessed directly
struct stringStack {
    int nStrings;
    int cap;
    struct string* strings;
};


struct string StringNew();
struct string StringSlice(struct string* orig, int start, int len);
void StringAppend(struct string* str, char c);
void StringSetLen(struct string* str, int len);
char StringGetLen(struct string str);
char StringGet(struct string str, int index);
char* StringGetPtr(struct string str);
struct stringStack StringStackNew();
void StringStackPush(struct stringStack* ss, struct string str);
struct string StringStackPeek(struct stringStack* ss, int index);


#endif //STR_H
