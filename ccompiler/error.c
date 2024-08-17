#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "color.h"
#include "token.h"


void Error(char* errMsg) {
    fputs(COLOR_RED "error: " COLOR_YELLOW, stdout);
    fputs(errMsg, stdout);
    puts(COLOR_RESET);
    exit(EXIT_FAILURE);
}


void CheckPtr(void* ptr) {
    if (!ptr) {
        Error("invalid pointer detected");
        exit(EXIT_FAILURE);
    }
}


static void SyntaxErrorBase(int line, char* fileName) {
    printf(COLOR_GREEN "%d " COLOR_RESET, line);
    printf(COLOR_CYAN "%s " COLOR_RESET, fileName);
    fputs(COLOR_RED "syntax error: " COLOR_RESET, stdout);
}


static void PrintChar(int c) {
    if (c == '\n') fputs("\\n", stdout);
    else putchar(c);
}


static void PrintCharIgnoreNewline(int c) {
    if (c != '\n') putchar(c);
}


static void PrintStr(struct str str) {
    for (int i = 0; i < StrGetLen(str); i++) {
        PrintChar(StrGetChar(str, i));
    }
}


static void SyntaxErrorLine(struct str line, int colStart, int colEnd) {
    fputs(COLOR_GREEN "-> " COLOR_BRIGHT_BLACK, stdout);
    for (int i = 0; i < colStart; i++) {
        PrintCharIgnoreNewline(StrGetChar(line, i));
    }

    fputs(COLOR_RED, stdout);
    for (int i = colStart; i <= colEnd; i++) {
        PrintChar(StrGetChar(line, i));
    }

    fputs(COLOR_BRIGHT_BLACK, stdout);
    for (int i = colEnd +1; i < line.len; i++) {
        PrintCharIgnoreNewline(StrGetChar(line, i));
    }
    puts(COLOR_RESET);
}


//reason may be NULL
void SyntaxErrorInvalidChar(struct tokenContext* tc, int col, char* reason) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    SyntaxErrorBase(tc->lines.nStrs, tc->fileName);
    fputs(COLOR_YELLOW, stdout);
    fputs("invalid character \"", stdout);
    PrintChar(StrGetChar(line, col));
    fputs("\"", stdout);
    if (reason) {
        fputs(" ", stdout);
        fputs(reason, stdout);
    }
    puts(COLOR_RESET);
    SyntaxErrorLine(line, col, col);
    exit(EXIT_FAILURE);
}


//reason may be NULL
void SyntaxErrorInvalidToken(struct token tok, char* reason) {
    struct tokenContext* tc = tok.context;
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    SyntaxErrorBase(tc->lines.nStrs, tc->fileName);
    fputs(COLOR_YELLOW, stdout);
    fputs("invalid token \"", stdout);
    PrintStr(tok.str);
    fputs("\"", stdout);
    if (reason) {
        fputs(" ", stdout);
        fputs(reason, stdout);
    }
    puts(COLOR_RESET);
    int colStart = StrGetPtr(tok.str) - StrGetPtr(line);
    SyntaxErrorLine(line, colStart, colStart + StrGetLen(tok.str) -1);
    exit(EXIT_FAILURE);
}
