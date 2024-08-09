#ifndef ERROR_H
#define ERROR_H
#include "token.h"

void Error(char* errMsg);
void CheckPtr(void* ptr);

//reason may be NULL
void SyntaxErrorInvalidChar(struct tokenContext* tc, int col, char* reason);

#endif //ERROR_H
