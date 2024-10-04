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


static void syntaxErrorBase(int line, struct str fileName) {
    printf(COLOR_GREEN "%d " COLOR_RESET, line);
    fputs(COLOR_CYAN, stdout);
    StrPrint(fileName, stdout);
    fputs(" " COLOR_RESET, stdout);
    fputs(COLOR_RED "syntax error: " COLOR_RESET, stdout);
}


static void printChar(int c) {
    if (c == '\n') fputs("\\n", stdout);
    else putchar(c);
}


static void printCharIgnoreNewline(int c) {
    if (c != '\n') putchar(c);
}


static void printStr(struct str str) {
    for (int i = 0; i < StrGetLen(str); i++) {
        printChar(StrGetChar(str, i));
    }
}


static void syntaxErrorLine(int lineNr, struct str line, int colStart, int colEnd) {
    printf(COLOR_GREEN "%d " COLOR_BRIGHT_BLACK, lineNr);
    for (int i = 0; i < colStart; i++) {
        printCharIgnoreNewline(StrGetChar(line, i));
    }

    fputs(COLOR_RED, stdout);
    for (int i = colStart; i <= colEnd; i++) {
        printChar(StrGetChar(line, i));
    }

    fputs(COLOR_BRIGHT_BLACK, stdout);
    for (int i = colEnd +1; i < line.len; i++) {
        printCharIgnoreNewline(StrGetChar(line, i));
    }
    puts(COLOR_RESET);
}


//reason may be NULL
void SyntaxErrorInvalidChar(struct tokenContext* tc, int col, char* reason) {
    struct str line = StrListGet(tc->lines, StrListLen(tc->lines) -1);
    syntaxErrorBase(StrListLen(tc->lines), tc->fileName);
    fputs(COLOR_YELLOW, stdout);
    fputs("\"", stdout);
    printChar(StrGetChar(line, col));
    fputs("\"", stdout);
    if (reason) {
        fputs(" ", stdout);
        fputs(reason, stdout);
    }
    puts(COLOR_RESET);
    syntaxErrorLine(StrListLen(tc->lines), line, col, col);
    exit(EXIT_FAILURE);
}


//reason may be NULL
void SyntaxErrorInvalidToken(struct token tok, char* reason) {
    struct tokenContext* tc = tok.context;
    struct str line = StrListGet(tc->lines, tok.lineNr -1);
    syntaxErrorBase(tok.lineNr, tc->fileName);
    fputs(COLOR_YELLOW, stdout);
    if (tok.type == TOKEN_EOF) {
        fputs("unexpected end of file", stdout);
    }
    else {
        fputs("\"", stdout);
        printStr(tok.str);
        fputs("\"", stdout);
    }
    if (reason) {
        fputs(" ", stdout);
        fputs(reason, stdout);
    }
    puts(COLOR_RESET);
    if (tok.type != TOKEN_EOF) {
        int colStart = StrGetSliceStrIndex(line, tok.str);
        syntaxErrorLine(tok.lineNr, line, colStart, colStart + StrGetLen(tok.str) -1);
    }
    exit(EXIT_FAILURE);
}
