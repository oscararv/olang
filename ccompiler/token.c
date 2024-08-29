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


struct token tokenPipePeek(struct tokenPipe* pipe) {
    if (pipe->nAvailable <= 0) Error("tried to peak into empty token pipe");
    return pipe->tokens[pipe->nDelivered];

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
    pipe->nAvailable++;
}


struct tokenContext TokenContextNew(char* fileName) {
    struct tokenContext tc;
    tc.fileName = strdup(fileName);
    CheckPtr(tc.fileName);

    tc.fp = fopen(fileName, "r");
    CheckPtr(tc.fileName);

    tc.tokens = TokenPipeNew();
    tc.lines = StrListNew();
    return tc;
}


void ParseComment(struct str line, int* col) {
    while (StrGetChar(line, *col) != '\n') (*col)++;
}


void FindTokenStart(struct str line, int* col) {
    char c = StrGetChar(line, *col);
    if (c == '#') ParseComment(line, col);
    while(c == ' ' || c == '\t') {
        (*col)++;
        c = StrGetChar(line, *col);
        if (c == '#') ParseComment(line, col);
    }
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
    if (StrEqualCharArray(tok->str, "import")) tok->type = TOKEN_IMPORT;
    else if (StrEqualCharArray(tok->str, "type")) tok->type = TOKEN_TYPE;
    else if (StrEqualCharArray(tok->str, "if")) tok->type = TOKEN_IF;
    else if (StrEqualCharArray(tok->str, "else")) tok->type = TOKEN_ELSE;
    else if (StrEqualCharArray(tok->str, "for")) tok->type = TOKEN_FOR;
    else if (StrEqualCharArray(tok->str, "defer")) tok->type = TOKEN_DEFER;
    else if (StrEqualCharArray(tok->str, "switch")) tok->type = TOKEN_SWITCH;
    else if (StrEqualCharArray(tok->str, "return")) tok->type = TOKEN_RETURN;
    else if (StrEqualCharArray(tok->str, "break")) tok->type = TOKEN_BREAK;
    else if (StrEqualCharArray(tok->str, "match")) tok->type = TOKEN_MATCH;
    else if (StrEqualCharArray(tok->str, "case")) tok->type = TOKEN_CASE;
    else if (StrEqualCharArray(tok->str, "struct")) tok->type = TOKEN_STRUCT;
    else if (StrEqualCharArray(tok->str, "vocab")) tok->type = TOKEN_VOCAB;
    else if (StrEqualCharArray(tok->str, "func")) tok->type = TOKEN_FUNC;
}


struct token ParseToken(struct tokenContext* tc, int* col) {
    struct str line = tc->lines.strs[tc->lines.nStrs -1];
    FindTokenStart(line, col);

    struct token tok;
    int colStart = *col;
    tok.lineNr = tc->lines.nStrs;
    tok.str = StrSlice(line, *col, *col);
    tok.context = tc;

    ParseTokenSwitch(tc, &tok, col);
    StrSetLen(&(tok.str), *col - colStart);

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
    StrListAppend(&(tc->lines), line);
    ValidateLatestLine(tc);

    int col= 0;
    while(col < StrGetLen(line)) {
        tokenPipePush(&(tc->tokens), ParseToken(tc, &col));
    }
    if (eof) {
        tokenPipePush(&(tc->tokens), TokenEOF(tc));
    }
}


struct token TokenPeek(struct tokenContext* tc) {
    if (tokenPipeIsEmpty(&(tc->tokens))) ParseLine(tc);
    return tokenPipePeek(&(tc->tokens));
}


struct token TokenNext(struct tokenContext* tc) {
    struct token tok;
    if (tokenPipeIsEmpty(&(tc->tokens))) ParseLine(tc);
    tok = tokenPipePop(&(tc->tokens));
    if (tok.type == TOKEN_EOF) tokenPipePush(&(tc->tokens), tok);
    return tok;
}


void TokenDiscardNewlines(struct tokenContext* tc) {
    struct token tok;
    while((tok = TokenNext(tc)).type == TOKEN_NEWLINE);
    tokenPipeUnpop(&(tc->tokens), tok);
}


struct token TokenNextDiscardNewlines(struct tokenContext* tc) {
    struct token tok;
    while((tok = TokenNext(tc)).type == TOKEN_NEWLINE);
    return tok;
}


void TokenUnget(struct tokenContext* tc, struct token tok) {
    tokenPipeUnpop(&(tc->tokens), tok);
}
