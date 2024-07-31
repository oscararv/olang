#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "color.h"
#include "token.h"


#define COLOR_ERROR_MSG COLOR_YELLOW


void Error(char* errMsg) {
    fputs(COLOR_RED "error: " COLOR_ERROR_MSG, stdout);
    fputs(errMsg, stdout);
    puts(COLOR_RESET);
}


void CheckPtr(void* ptr) {
    if (!ptr) Error("invalid pointer detected");
    exit(EXIT_FAILURE);
}



static void SyntaxErrorBase(int line, char* fileName) {
    printf(COLOR_GREEN "%d " COLOR_RESET, line);
    printf(COLOR_CYAN "%s " COLOR_RESET, fileName);
    fputs(COLOR_RED "syntax error: " COLOR_RESET, stdout);
}


static void SyntaxErrorLine(struct stringStack* ss, int colStart, int colEnd) {
    struct string str = ss->strings[ss->nStrings -1];
    for (int i = 0; i < colStart; i++) {
        putchar(str.ptr[i]);
    }

    fputs(COLOR_RED, stdout);
    for (int i = colStart; i <= colEnd;) {
        putchar(str.ptr[i]);
    }

    fputs(COLOR_RESET, stdout);
    for (int i = colEnd +1; i < str.len; i++) {
        putchar(str.ptr[i]);
    }
}


void SyntaxErrorInvalidChar(struct tokenContext* tc, char c, int col, char* reason) {
    (void)col;
    SyntaxErrorBase(tc->lines.nStrings, tc->fileName);
    printf(COLOR_ERROR_MSG "invalid character \"%c\" ", c);
    fputs(reason, stdout);
    puts(COLOR_RESET);
    SyntaxErrorLine(&(tc->lines), col, col);
}
