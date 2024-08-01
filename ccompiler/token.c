#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "error.h"
#include "token.h"


struct tokenPipe TokenPipeNew() {
    struct tokenPipe tp;
    tp.nAvailable = 0;
    tp.nDelivered = 0;
    tp.tokens = NULL;
    return tp;
}


bool tokenPipeIsEmpty(struct tokenPipe* pipe) {
    return pipe->nAvailable > 0;
}


void tokenPipePush(struct tokenPipe* pipe, struct token tok) {
    if (pipe->nAvailable + pipe->nDelivered >= pipe->cap) {
        pipe->cap += 1000;
        pipe->tokens = realloc(pipe->tokens, sizeof(struct token) * pipe->cap);
        CheckPtr(pipe->tokens);
    }

    pipe->tokens[pipe->nDelivered + pipe->nAvailable] = tok;
    pipe->nAvailable++;
}


struct token tokenPipePop(struct tokenPipe* pipe) {
    if (pipe->nAvailable <= 0) Error("tried to pop from empty token pipe");
    pipe->nDelivered++;
    pipe->nAvailable--;
    return pipe->tokens[pipe->nDelivered -1];
}


void tokenPipeUnpop(struct tokenPipe* pipe, struct token tok) {
    if (pipe->nDelivered <= 0) Error("tried to unpop to pipe with no deliveries");
    pipe->nDelivered--;
    pipe->tokens[pipe->nDelivered] = tok;
}


struct string StringNew() {
    struct string str;
    str.len = 0;
    str.cap = 0;
    str.ptr = NULL;
    return str;
}


void StringAppend(struct string* str, char c) {
    if (str->len >= str->cap) {
        str->cap += 100;
        str->ptr = realloc(str->ptr, sizeof(char) * str->cap);
        CheckPtr(str->ptr);
    }

    str->ptr[str->len] = c;
    str->len++;
}


char StringGet(struct string str, int index) {
    if (index >= str.len) Error("index out of bounds when reading string");
    return str.ptr[index];
}


struct stringStack StringStackNew() {
    struct stringStack ss;
    ss.nStrings = 0;
    ss.cap = 0;
    ss.strings = NULL;
    return ss;
}


void StringStackPush(struct stringStack* ss, struct string str) {
    if (ss->nStrings >= ss->cap) {
        ss->cap += 100;
        ss->strings = realloc(ss->strings, sizeof(struct string));
        CheckPtr(ss->strings);
    }

    ss->strings[ss->nStrings] = str;
    ss->nStrings++;
}


struct string StringStackPeek(struct stringStack* ss, int index) {
    if (index >= ss->nStrings) Error("tried to peek into string stack at invalid index");
    return ss->strings[index];
}


struct tokenContext TokenContextNew(char* fileName) {
    struct tokenContext tc;
    tc.fileName = strdup(fileName);
    CheckPtr(tc.fileName);

    tc.fp = fopen(fileName, "r");
    CheckPtr(tc.fileName);

    tc.tokens = TokenPipeNew();
    tc.lines = StringStackNew();
    return tc;
}


void ParseComment(struct tokenContext* tc, struct token* tok, int* colStart) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    char c;
    while((c = StringGet(line, *colStart)) != '\n') {
        StringAppend(&(tok->str), c);
        (*colStart)++;
    }
}


void ParseTokenSwitch(struct tokenContext* tc, struct token* tok, int* colStart) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    char c = line.ptr[*colStart];
    switch (c) {
        case '#': tok->type = TOKEN_COMMENT; ParseComment(tc, tok, colStart); break;
        default: SyntaxErrorInvalidChar(tc, c, tc->lines.nStrings -1, *colStart, NULL);
    }
}


int FindNonSpaceNonTab(struct string str, int colStart) {
    int i = colStart;
    while(str.ptr[i] == '\n' || str.ptr[i] == '\t') i++;
    return i;
}


struct token ParseToken(struct tokenContext* tc, int* colStart) {
    struct token tok;
    tok.line = tc->lines.nStrings;
    tok.str = StringNew();
    tok.context = tc;

    *colStart = FindNonSpaceNonTab(tc->lines.strings[tc->lines.nStrings -1], *colStart);
    tok.colStart = *colStart;

    ParseTokenSwitch(tc, &tok, colStart);
    tok.colEnd = *colStart -1;
    return tok;
}


struct string ReadLine(FILE* fp) {
    struct string str = StringNew();

    char c;
    while((c = fgetc(fp)) != '\n') {
        StringAppend(&str, c);
    }
    return str;
}


void ParseLine(struct tokenContext* tc) {
    struct string str = ReadLine(tc->fp);
    StringStackPush(&(tc->lines), str);

    int colStart = 0;
    while(colStart < str.len) {
        tokenPipePush(&(tc->tokens), ParseToken(tc, &colStart));
    }
}


struct token TokenNext(struct tokenContext* tc) {
    if (tokenPipeIsEmpty(&(tc->tokens))) {
        ParseLine(tc);
    }

    struct token tok = tokenPipePop(&(tc->tokens));
    if (tok.type == TOKEN_EOF) tokenPipePush(&(tc->tokens), tok);
    return tok;
}


struct token TokenNextDiscardNewlines(struct tokenContext* tc) {
    struct token tok;
    while((tok = TokenNext(tc)).type == TOKEN_NEWLINE);
    return tok;
}
