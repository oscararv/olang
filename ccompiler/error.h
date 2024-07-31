#ifndef ERROR_H
#define ERROR_H
#include "token.h"

void Error(char* errMsg);
void CheckPtr(void* ptr);
void SyntaxErrorInvalidChar(struct tokenContext* tc, char c, int row, int col, char* reason);

#endif //ERROR_H
