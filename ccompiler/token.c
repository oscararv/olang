#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "token.h"
#include "error.h"
#include "str.h"


struct tokenPipe TokenPipeNew() {
    struct tokenPipe tp;
    tp.nAvailable = 0;
    tp.nDelivered = 0;
    tp.tokens = NULL;
    return tp;
}


bool tokenPipeIsEmpty(struct tokenPipe* pipe) {
    return pipe->nAvailable <= 0;
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


int FindNonSpaceNonTab(struct string str, int col) {
    int i = col;
    while(str.ptr[i] == ' ' || str.ptr[i] == '\t') i++;
    return i;
}


bool IsDigit(char c) {
    if (c >= '0' && c <= '9') return true;
    return false;
}


bool IsIdentifier(char c) {
    if (IsDigit(c)) return true;
    if (c >= 'A' && c <= 'Z') return true;
    if (c >= 'a' && c <= 'z') return true;
    if (c == '_') return true;
    return false;
}


void ParseComment(struct tokenContext* tc, int* col) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    char c;
    while ((c = StringGet(line, *col)) != '\n') (*col)++;
}


void ParseIdentifier(struct tokenContext* tc, int* col) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    while (IsIdentifier(StringGet(line, *col))) (*col)++;
}


enum tokenType ParseNumber(struct tokenContext* tc, int* col) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    int nDots = 0;
    while (IsDigit(StringGet(line, *col)) || StringGet(line, *col) == '.') {
        if (StringGet(line, *col) == '.') {
            nDots++;
            if (nDots > 1) SyntaxErrorInvalidChar(tc, *col,
                        "numbers may not contain more than one decimal point");
        }
        (*col)++;
    }
    if (StringGet(line, *col-1) == '.') SyntaxErrorInvalidChar(tc,
            *col, "numbers may not end in a decimal point");

    if (nDots == 0) return TOKEN_INT;
    else return TOKEN_FLOAT;
}


static bool IsEscapeChar(char c, bool string) {
    if (c == 'n') return true;
    if (c == 't') return true;
    if (c == '\\') return true;
    if (c == '"' && string) return true;
    if (c == '\'' && !string) return true;
    return false;
}


static void ParseChar(struct tokenContext* tc, int* col, bool inString) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    char c = StringGet(line, *col);
    if (c == '\n') SyntaxErrorInvalidChar(tc, *col, NULL);
    if (c == '\\') {
        (*col)++;
        if (!IsEscapeChar(StringGet(line, *col), inString)) {
            SyntaxErrorInvalidChar(tc, *col, "invalid escape character");
        }
    }
    (*col)++;
}


void ParseCharConstant(struct tokenContext* tc, int* col) {
    ParseChar(tc, col, false);
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    if (StringGet(line, *col) != '\'') SyntaxErrorInvalidChar(tc, *col,
            "character constants may only contain one character");
    (*col)++;
}


void ParseStringConstant(struct tokenContext* tc, int* col) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    char c;
    while ((c = StringGet(line, *col)) != '"') {
        ParseChar(tc, col, true);
    }
    (*col)++;
}


void ParseTokenSwitch(struct tokenContext* tc, struct token* tok, int* col) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    char c = line.ptr[*col];
    (*col)++;

    if (IsDigit(c)) tok->type = ParseNumber(tc, col);
    else if (IsIdentifier(c)) {
        tok->type = TOKEN_IDENTIFIER;
        ParseIdentifier(tc, col);
    }
    else switch (c) {
        case '#': tok->type = TOKEN_COMMENT; ParseComment(tc, col); break;
        case '\n': tok->type = TOKEN_NEWLINE; break;
        case '\'': tok->type = TOKEN_CHAR; ParseCharConstant(tc, col); break;
        case '"': tok->type = TOKEN_STRING; ParseStringConstant(tc, col); break;
        default: SyntaxErrorInvalidChar(tc, *col-1, NULL); break;
    }
}


static void SpecifyIdentifier(struct token* tok) {
    if (strncmp(StringGetPtr(tok->str), "import", StringGetLen(tok->str))) tok->type = TOKEN_IMPORT;
    if (strncmp(StringGetPtr(tok->str), "type", StringGetLen(tok->str))) tok->type = TOKEN_TYPE;
    if (strncmp(StringGetPtr(tok->str), "if", StringGetLen(tok->str))) tok->type = TOKEN_IF;
    if (strncmp(StringGetPtr(tok->str), "else", StringGetLen(tok->str))) tok->type = TOKEN_ELSE;
    if (strncmp(StringGetPtr(tok->str), "for", StringGetLen(tok->str))) tok->type = TOKEN_FOR;
    if (strncmp(StringGetPtr(tok->str), "defer", StringGetLen(tok->str))) tok->type = TOKEN_DEFER;
    if (strncmp(StringGetPtr(tok->str), "switch", StringGetLen(tok->str))) tok->type = TOKEN_SWITCH;
    if (strncmp(StringGetPtr(tok->str), "return", StringGetLen(tok->str))) tok->type = TOKEN_RETURN;
    if (strncmp(StringGetPtr(tok->str), "break", StringGetLen(tok->str))) tok->type = TOKEN_BREAK;
    if (strncmp(StringGetPtr(tok->str), "match", StringGetLen(tok->str))) tok->type = TOKEN_MATCH;
    if (strncmp(StringGetPtr(tok->str), "case", StringGetLen(tok->str))) tok->type = TOKEN_CASE;
    if (strncmp(StringGetPtr(tok->str), "bool", StringGetLen(tok->str))) tok->type = TOKEN_BOOL;
    if (strncmp(StringGetPtr(tok->str), "byte", StringGetLen(tok->str))) tok->type = TOKEN_BYTE;
    if (strncmp(StringGetPtr(tok->str), "int8", StringGetLen(tok->str))) tok->type = TOKEN_INT8;
    if (strncmp(StringGetPtr(tok->str), "int16", StringGetLen(tok->str))) tok->type = TOKEN_INT16;
    if (strncmp(StringGetPtr(tok->str), "int32", StringGetLen(tok->str))) tok->type = TOKEN_INT32;
    if (strncmp(StringGetPtr(tok->str), "int64", StringGetLen(tok->str))) tok->type = TOKEN_INT64;
    if (strncmp(StringGetPtr(tok->str), "uint8", StringGetLen(tok->str))) tok->type = TOKEN_UINT8;
    if (strncmp(StringGetPtr(tok->str), "uint16", StringGetLen(tok->str))) tok->type = TOKEN_UINT16;
    if (strncmp(StringGetPtr(tok->str), "uint32", StringGetLen(tok->str))) tok->type = TOKEN_UINT32;
    if (strncmp(StringGetPtr(tok->str), "uint64", StringGetLen(tok->str))) tok->type = TOKEN_UINT64;
    if (strncmp(StringGetPtr(tok->str), "float32", StringGetLen(tok->str))) tok->type = TOKEN_FLOAT32;
    if (strncmp(StringGetPtr(tok->str), "float64", StringGetLen(tok->str))) tok->type = TOKEN_FLOAT64;
    if (strncmp(StringGetPtr(tok->str), "struct", StringGetLen(tok->str))) tok->type = TOKEN_STRUCT;
    if (strncmp(StringGetPtr(tok->str), "vocab", StringGetLen(tok->str))) tok->type = TOKEN_VOCAB;
    if (strncmp(StringGetPtr(tok->str), "func", StringGetLen(tok->str))) tok->type = TOKEN_FUNC;
}


struct token ParseToken(struct tokenContext* tc, int* col) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    *col= FindNonSpaceNonTab(line, *col);

    struct token tok;
    tok.lineNr = tc->lines.nStrings;
    tok.str = StringSlice(&line, *col, *col);
    tok.context = tc;

    ParseTokenSwitch(tc, &tok, col);
    StringSetLen(&(tok.str), *col);

    if (tok.type == TOKEN_IDENTIFIER) SpecifyIdentifier(&tok);
    return tok;
}


struct string ReadLine(FILE* fp, bool* eof) {
    struct string str = StringNew();

    while(true) {
        int c = fgetc(fp);
        if (c == EOF) {*eof = true; break;}
        StringAppend(&str, (char)c);
        if (c == '\n') break;
    }
    return str;
}


struct token TokenEOF(struct tokenContext* tc) {
    struct token tok;
    tok.type = TOKEN_EOF;
    tok.lineNr = tc->lines.nStrings;
    tok.str = StringNew();
    tok.context = tc;
    return tok;
}


bool IsValidChar(char c) {
    if (c < ' ') return false;
    if (c == 127) return false;
    return true;
}


void ValidateLatestLine(struct tokenContext* tc) {
    struct string line = tc->lines.strings[tc->lines.nStrings -1];
    for (int i = 0; i < line.len -1; i++) {
        if (!IsValidChar(StringGet(line, i))) SyntaxErrorInvalidChar(tc, i, NULL);
    }
}


void ParseLine(struct tokenContext* tc) {
    bool eof = false;
    struct string line = ReadLine(tc->fp, &eof);
    StringStackPush(&(tc->lines), line);
    ValidateLatestLine(tc);

    int col= 0;
    while(col< line.len) {
        tokenPipePush(&(tc->tokens), ParseToken(tc, &col));
    }
    if (eof) {
        tokenPipePush(&(tc->tokens), TokenEOF(tc));
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
