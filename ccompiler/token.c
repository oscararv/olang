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
    tc.lines = StrStackNew();
    return tc;
}


int FindNonSpaceNonTab(struct str str, int col) {
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
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    char c;
    while ((c = StrGetChar(line, *col)) != '\n') (*col)++;
}


void ParseIdentifier(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    while (IsIdentifier(StrGetChar(line, *col))) (*col)++;
}


enum tokenType ParseNumber(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    int nDots = 0;
    while (IsDigit(StrGetChar(line, *col)) || StrGetChar(line, *col) == '.') {
        if (StrGetChar(line, *col) == '.') {
            nDots++;
            if (nDots > 1) SyntaxErrorInvalidChar(tc, *col,
                        "numbers may not contain more than one decimal point");
        }
        (*col)++;
    }
    if (StrGetChar(line, *col-1) == '.') SyntaxErrorInvalidChar(tc,
            *col, "numbers may not end in a decimal point");

    if (nDots == 0) return TOKEN_INT;
    else return TOKEN_FLOAT;
}


static bool IsEscapeChar(char c, bool inString) {
    if (c == 'n') return true;
    if (c == 't') return true;
    if (c == '\\') return true;
    if (c == '"' && inString) return true;
    if (c == '\'' && !inString) return true;
    return false;
}


static void ParseChar(struct tokenContext* tc, int* col, bool inStr) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    char c = StrGetChar(line, *col);
    if (c == '\n') SyntaxErrorInvalidChar(tc, *col, NULL);
    if (c == '\\') {
        (*col)++;
        if (!IsEscapeChar(StrGetChar(line, *col), inStr)) {
            SyntaxErrorInvalidChar(tc, *col, "invalid escape character");
        }
    }
    (*col)++;
}


void ParseCharConstant(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    if (StrGetChar(line, *col) == '\'') SyntaxErrorInvalidChar(tc, *col,
            "character constants may not be empty");
    ParseChar(tc, col, false);
    if (StrGetChar(line, *col) != '\'') SyntaxErrorInvalidChar(tc, *col,
            "character constants may only contain one character");
    (*col)++;
}


void ParseStringConstant(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    char c;
    while ((c = StrGetChar(line, *col)) != '"') {
        ParseChar(tc, col, true);
    }
    (*col)++;
}


enum tokenType ParseEqualSign(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_LOGICAL_EQUALS;
    }
    return TOKEN_ASSIGNMENT;
}


enum tokenType ParseAmpersand(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    if (StrGetChar(line, *col) == '&') {
        (*col)++;
        return TOKEN_LOGICAL_AND;
    }
    return TOKEN_BITWISE_AND;
}


enum tokenType ParsePipe(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    if (StrGetChar(line, *col) == '|') {
        (*col)++;
        return TOKEN_LOGICAL_OR;
    }
    return TOKEN_BITWISE_OR;
}


enum tokenType ParseLessThan(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    char c = StrGetChar(line, *col);
    if (c == '<') {
        (*col)++;
        return TOKEN_BITSHIFT_LEFT;
    }
    if (c == '=') {
        (*col)++;
        return TOKEN_LOGICAL_LESS_THAN_OR_EQUAL;
    }
    return TOKEN_LOGICAL_LESS_THAN;
}


enum tokenType ParseGreaterThan(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    char c = StrGetChar(line, *col);
    if (c == '>') {
        (*col)++;
        return TOKEN_BITSHIFT_RIGHT;
    }
    if (c == '=') {
        (*col)++;
        return TOKEN_LOGICAL_GREATER_THAN_OR_EQUAL;
    }
    return TOKEN_LOGICAL_GREATER_THAN;
}


enum tokenType ParsePlusSign(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    char c = StrGetChar(line, *col);
    if (c == '+') {
        (*col)++;
        return TOKEN_INCREMENT;
    }
    if (c == '=') {
        (*col)++;
        return TOKEN_ASSIGNMENT_ADD;
    }
    return TOKEN_ADD;
}


enum tokenType ParseHyphen(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    char c = StrGetChar(line, *col);
    if (c == '-') {
        (*col)++;
        return TOKEN_DECREMENT;
    }
    if (c == '=') {
        (*col)++;
        return TOKEN_ASSIGNMENT_SUB;
    }
    return TOKEN_SUB;
}


enum tokenType ParseAsterisk(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_ASSIGNMENT_MUL;
    }
    return TOKEN_MUL;
}


enum tokenType ParseForwardSlash(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_ASSIGNMENT_DIV;
    }
    return TOKEN_DIV;
}


enum tokenType ParsePercentSign(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_ASSIGNMENT_MODULO;
    }
    return TOKEN_MODULO;
}


void ParseTokenSwitch(struct tokenContext* tc, struct token* tok, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
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
        case '=': tok->type = ParseEqualSign(tc, col); break;
        case '&': tok->type = ParseAmpersand(tc, col); break;
        case '|': tok->type = ParsePipe(tc, col); break;
        case '<': tok->type = ParseLessThan(tc, col); break;
        case '>': tok->type = ParseGreaterThan(tc, col); break;
        case '+': tok->type = ParsePlusSign(tc, col); break;
        case '-': tok->type = ParseHyphen(tc, col); break;
        case '*': tok->type = ParseAsterisk(tc, col); break;
        case '/': tok->type = ParseForwardSlash(tc, col); break;
        case '%': tok->type = ParsePercentSign(tc, col); break;

        case '.': tok->type = TOKEN_DOT; break;
        case ',': tok->type = TOKEN_COMMA; break;
        case '(': tok->type = TOKEN_PAREN_OPEN; break;
        case ')': tok->type = TOKEN_PAREN_CLOSE; break;
        case '[': tok->type = TOKEN_SQUAREBRACKET_OPEN; break;
        case ']': tok->type = TOKEN_SQUAREBRACKET_CLOSE; break;
        case '{': tok->type = TOKEN_CURLYBRACKET_OPEN; break;
        case '}': tok->type = TOKEN_CURLYBRACKET_CLOSE; break;
        case '!': tok->type = TOKEN_LOGICAL_NOT; break;
        case '^': tok->type = TOKEN_BITWISE_XOR; break;
        case '~': tok->type = TOKEN_BITWISE_COMPLEMENT; break;
        default: SyntaxErrorInvalidChar(tc, *col-1, NULL); break;
    }
}


static void SpecifyIdentifier(struct token* tok) {
    if (strncmp(StrGetPtr(tok->str), "import", StrGetLen(tok->str))) tok->type = TOKEN_IMPORT;
    if (strncmp(StrGetPtr(tok->str), "type", StrGetLen(tok->str))) tok->type = TOKEN_TYPE;
    if (strncmp(StrGetPtr(tok->str), "if", StrGetLen(tok->str))) tok->type = TOKEN_IF;
    if (strncmp(StrGetPtr(tok->str), "else", StrGetLen(tok->str))) tok->type = TOKEN_ELSE;
    if (strncmp(StrGetPtr(tok->str), "for", StrGetLen(tok->str))) tok->type = TOKEN_FOR;
    if (strncmp(StrGetPtr(tok->str), "defer", StrGetLen(tok->str))) tok->type = TOKEN_DEFER;
    if (strncmp(StrGetPtr(tok->str), "switch", StrGetLen(tok->str))) tok->type = TOKEN_SWITCH;
    if (strncmp(StrGetPtr(tok->str), "return", StrGetLen(tok->str))) tok->type = TOKEN_RETURN;
    if (strncmp(StrGetPtr(tok->str), "break", StrGetLen(tok->str))) tok->type = TOKEN_BREAK;
    if (strncmp(StrGetPtr(tok->str), "match", StrGetLen(tok->str))) tok->type = TOKEN_MATCH;
    if (strncmp(StrGetPtr(tok->str), "case", StrGetLen(tok->str))) tok->type = TOKEN_CASE;
    if (strncmp(StrGetPtr(tok->str), "bool", StrGetLen(tok->str))) tok->type = TOKEN_BOOL;
    if (strncmp(StrGetPtr(tok->str), "byte", StrGetLen(tok->str))) tok->type = TOKEN_BYTE;
    if (strncmp(StrGetPtr(tok->str), "int8", StrGetLen(tok->str))) tok->type = TOKEN_INT8;
    if (strncmp(StrGetPtr(tok->str), "int16", StrGetLen(tok->str))) tok->type = TOKEN_INT16;
    if (strncmp(StrGetPtr(tok->str), "int32", StrGetLen(tok->str))) tok->type = TOKEN_INT32;
    if (strncmp(StrGetPtr(tok->str), "int64", StrGetLen(tok->str))) tok->type = TOKEN_INT64;
    if (strncmp(StrGetPtr(tok->str), "uint8", StrGetLen(tok->str))) tok->type = TOKEN_UINT8;
    if (strncmp(StrGetPtr(tok->str), "uint16", StrGetLen(tok->str))) tok->type = TOKEN_UINT16;
    if (strncmp(StrGetPtr(tok->str), "uint32", StrGetLen(tok->str))) tok->type = TOKEN_UINT32;
    if (strncmp(StrGetPtr(tok->str), "uint64", StrGetLen(tok->str))) tok->type = TOKEN_UINT64;
    if (strncmp(StrGetPtr(tok->str), "float32", StrGetLen(tok->str))) tok->type = TOKEN_FLOAT32;
    if (strncmp(StrGetPtr(tok->str), "float64", StrGetLen(tok->str))) tok->type = TOKEN_FLOAT64;
    if (strncmp(StrGetPtr(tok->str), "struct", StrGetLen(tok->str))) tok->type = TOKEN_STRUCT;
    if (strncmp(StrGetPtr(tok->str), "vocab", StrGetLen(tok->str))) tok->type = TOKEN_VOCAB;
    if (strncmp(StrGetPtr(tok->str), "func", StrGetLen(tok->str))) tok->type = TOKEN_FUNC;
}


struct token ParseToken(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    *col= FindNonSpaceNonTab(line, *col);

    struct token tok;
    tok.lineNr = tc->lines.nStrs;
    tok.str = StrSlice(line, *col, *col);
    tok.context = tc;

    ParseTokenSwitch(tc, &tok, col);
    StrSetLen(&(tok.str), *col);

    if (tok.type == TOKEN_IDENTIFIER) SpecifyIdentifier(&tok);
    return tok;
}


struct str ReadLine(FILE* fp, bool* eof) {
    struct str str = StrNew();

    while(true) {
        int c = fgetc(fp);
        if (c == EOF) {
            *eof = true;
            c = '\n';
        }
        StrAppend(&str, (char)c);
        if (c == '\n') break;
    }
    return str;
}


struct token TokenEOF(struct tokenContext* tc) {
    struct token tok;
    tok.type = TOKEN_EOF;
    tok.lineNr = tc->lines.nStrs;
    tok.str = StrNew();
    tok.context = tc;
    return tok;
}


bool IsValidChar(char c) {
    if (c < ' ') return false;
    if (c == 127) return false;
    return true;
}


void ValidateLatestLine(struct tokenContext* tc) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    for (int i = 0; i < line.len -1; i++) {
        if (!IsValidChar(StrGetChar(line, i))) SyntaxErrorInvalidChar(tc, i, NULL);
    }
}


void ParseLine(struct tokenContext* tc) {
    bool eof = false;
    struct str line = ReadLine(tc->fp, &eof);
    StrStackPush(&(tc->lines), line);
    ValidateLatestLine(tc);

    int col= 0;
    while(col < StrGetLen(line)) {
        tokenPipePush(&(tc->tokens), ParseToken(tc, &col));
    }
    if (eof) {
        tokenPipePush(&(tc->tokens), TokenEOF(tc));
    }
}


struct token TokenNext(struct tokenContext* tc) {
    struct token tok;
    do {
        if (tokenPipeIsEmpty(&(tc->tokens))) {
            ParseLine(tc);
        }
        tok = tokenPipePop(&(tc->tokens));
        if (tok.type == TOKEN_EOF) tokenPipePush(&(tc->tokens), tok);
    }
    while (tok.type == TOKEN_COMMENT);
    return tok;
}


struct token TokenNextDiscardNewlines(struct tokenContext* tc) {
    struct token tok;
    while((tok = TokenNext(tc)).type == TOKEN_NEWLINE);
    return tok;
}
