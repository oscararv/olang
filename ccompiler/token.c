#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "token.h"
#include "error.h"
#include "str.h"


struct tokenList TokenListNew() {
    return (struct tokenList){0};
}


void TokenListAppend(struct tokenList* tl, struct token tok) {
    if (tl->isSlice) Error("list slices may not be appended");
    if (tl->len >= tl->cap) {
        tl->cap += 100;
        tl->ptr = realloc(tl->ptr, sizeof(struct token)* tl->cap);
        CheckPtr(tl->ptr);
    }
    tl->ptr[tl->len] = tok;
    tl->len++;
}


struct tokenList TokenListSlice(struct tokenList orig, int start, int stop) {
    if (stop > orig.len) Error("string slice stop index is greater than original string len");
    if (start < 0) Error("string slice start index is less than zero");
    orig.isSlice = true;
    orig.len = stop - start;
    orig.ptr = orig.ptr + start;
    return orig;
}


struct tokenPipe TokenPipeNew() {
    return (struct tokenPipe){0};
}


bool tokenPipeIsEmpty(struct tokenPipe* pipe) {
    return pipe->nAvailable <= 0;
}


void tokenPipePush(struct tokenPipe* pipe, struct token tok) {
    if (pipe->nAvailable + pipe->nDelivered >= pipe->cap) {
        pipe->cap += 1000;
        pipe->ptr = realloc(pipe->ptr, sizeof(*(pipe->ptr)) * pipe->cap);
        CheckPtr(pipe->ptr);
    }

    pipe->ptr[pipe->nDelivered + pipe->nAvailable] = tok;
    pipe->nAvailable++;
}


struct token tokenPipePop(struct tokenPipe* pipe) {
    if (pipe->nAvailable <= 0) Error("tried to pop from empty token pipe");
    pipe->nDelivered++;
    pipe->nAvailable--;
    return pipe->ptr[pipe->nDelivered -1];
}


struct token tokenPipePeek(struct tokenPipe* pipe) {
    if (pipe->nAvailable <= 0) Error("tried to peek into empty token pipe");
    return pipe->ptr[pipe->nDelivered];
}


void tokenPipeRestart(struct tokenPipe* pipe) {
    pipe->nAvailable += pipe->nDelivered;
    pipe->nDelivered = 0;
}


void tokenPipeUnpop(struct tokenPipe* pipe, int n) {
    if (pipe->nDelivered - n < 0) Error("tried to unpop to pipe with no deliveries");
    pipe->nDelivered-=n;
    pipe->nAvailable+=n;
}


struct tokenContext* TokenContextNew(struct str fileName) {
    struct tokenContext* tc = malloc(sizeof(*tc));
    CheckPtr(tc);
    tc->fileName = fileName;

    char charArrName[StrGetLen(fileName) +1];
    StrToCharArray(fileName, charArrName);
    tc->fp = fopen(charArrName, "r");
    if (!tc->fp) {
        char errorMsg[StrGetLen(fileName) + 100];
        errorMsg[0] = '"';
        StrToCharArray(fileName, errorMsg+1);
        strcat(errorMsg, "\" could not be opened");
        Error(errorMsg);
    }

    tc->tokens = TokenPipeNew();
    tc->lines = StrListNew();
    return tc;
}


bool FindTokenStart(struct str line, int* col) {
    char c = StrGetChar(line, *col);
    while(c == ' ' || c == '\t' || c == '\n' || c == '#') {
        if (c == '\n') return false;
        if (c == '#') return false;
        (*col)++;
        c = StrGetChar(line, *col);
    }
    return true;
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
    struct str line = StrListGetLast(tc->lines);
    while (IsIdentifier(StrGetChar(line, *col))) (*col)++;
}


enum tokenType ParseNumber(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
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
    struct str line = StrListGetLast(tc->lines);
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


void ParseCharLiteral(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
    if (StrGetChar(line, *col) == '\'') SyntaxErrorInvalidChar(tc, *col,
            "character literals may not be empty");
    ParseChar(tc, col, false);
    if (StrGetChar(line, *col) != '\'') SyntaxErrorInvalidChar(tc, *col,
            "character literals may only contain one character");
    (*col)++;
}


void ParseStringLiteral(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
    char c;
    while ((c = StrGetChar(line, *col)) != '"') {
        ParseChar(tc, col, true);
    }
    (*col)++;
}


enum tokenType ParseEqualSign(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_LOGICAL_EQUALS;
    }
    return TOKEN_ASSIGNMENT;
}


enum tokenType ParseExclamationMark(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_LOGICAL_NOT_EQUALS;
    }
    return TOKEN_LOGICAL_NOT;
}


enum tokenType ParseAmpersand(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
    if (StrGetChar(line, *col) == '&') {
        (*col)++;
        return TOKEN_LOGICAL_AND;
    }
    return TOKEN_BITWISE_AND;
}


enum tokenType ParsePipe(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
    if (StrGetChar(line, *col) == '|') {
        (*col)++;
        return TOKEN_LOGICAL_OR;
    }
    return TOKEN_BITWISE_OR;
}


enum tokenType ParseLessThan(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
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
    struct str line = StrListGetLast(tc->lines);
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
    struct str line = StrListGetLast(tc->lines);
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
    struct str line = StrListGetLast(tc->lines);
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
    struct str line = StrListGetLast(tc->lines);
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_ASSIGNMENT_MUL;
    }
    return TOKEN_MUL;
}


enum tokenType ParseForwardSlash(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_ASSIGNMENT_DIV;
    }
    return TOKEN_DIV;
}


enum tokenType ParsePercentSign(struct tokenContext* tc, int* col) {
    struct str line = StrListGetLast(tc->lines);
    if (StrGetChar(line, *col) == '=') {
        (*col)++;
        return TOKEN_ASSIGNMENT_MODULO;
    }
    return TOKEN_MODULO;
}


void ParseTokenSwitch(struct tokenContext* tc, struct token* tok, int* col) {
    struct str line = StrListGetLast(tc->lines);
    char c = line.ptr[*col];
    (*col)++;

    if (IsDigit(c)) tok->type = ParseNumber(tc, col);
    else if (IsIdentifier(c)) {
        tok->type = TOKEN_IDENTIFIER;
        ParseIdentifier(tc, col);
    }
    else switch (c) {
        case '\'': tok->type = TOKEN_CHAR; ParseCharLiteral(tc, col); break;
        case '"': tok->type = TOKEN_STRING; ParseStringLiteral(tc, col); break;
        case '=': tok->type = ParseEqualSign(tc, col); break;
        case '!': tok->type = ParseExclamationMark(tc, col); break;
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
        case '^': tok->type = TOKEN_BITWISE_XOR; break;
        case '~': tok->type = TOKEN_BITWISE_COMPLEMENT; break;
        default: SyntaxErrorInvalidChar(tc, *col-1, "illegal character"); break;
    }
}


static void SpecifyIdentifier(struct token* tok) {
    if (StrEqualCharArray(tok->str, "import")) tok->type = TOKEN_IMPORT;
    else if (StrEqualCharArray(tok->str, "type")) tok->type = TOKEN_TYPE;
    else if (StrEqualCharArray(tok->str, "if")) tok->type = TOKEN_IF;
    else if (StrEqualCharArray(tok->str, "else")) tok->type = TOKEN_ELSE;
    else if (StrEqualCharArray(tok->str, "for")) tok->type = TOKEN_FOR;
    else if (StrEqualCharArray(tok->str, "defer")) tok->type = TOKEN_DEFER;
    else if (StrEqualCharArray(tok->str, "return")) tok->type = TOKEN_RETURN;
    else if (StrEqualCharArray(tok->str, "break")) tok->type = TOKEN_BREAK;
    else if (StrEqualCharArray(tok->str, "match")) tok->type = TOKEN_MATCH;
    else if (StrEqualCharArray(tok->str, "case")) tok->type = TOKEN_CASE;
    else if (StrEqualCharArray(tok->str, "struct")) tok->type = TOKEN_STRUCT;
    else if (StrEqualCharArray(tok->str, "vocab")) tok->type = TOKEN_VOCAB;
    else if (StrEqualCharArray(tok->str, "func")) tok->type = TOKEN_FUNC;
    else if (StrEqualCharArray(tok->str, "mut")) tok->type = TOKEN_MUT;
}


bool ParseToken(struct tokenContext* tc, int* col, struct token* tok) {
    struct str line = StrListGetLast(tc->lines);
    if (!FindTokenStart(line, col)) return false;

    int colStart = *col;
    tok->lineNr = StrListLen(tc->lines);
    tok->str = StrSlice(line, *col, *col);
    tok->context = tc;

    ParseTokenSwitch(tc, tok, col);
    StrSetLen(&(tok->str), *col - colStart);

    if (tok->type == TOKEN_IDENTIFIER) SpecifyIdentifier(tok);
    return true;
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
    tok.lineNr = StrListLen(tc->lines);
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
    struct str line = StrListGetLast(tc->lines);
    for (int i = 0; i < line.len -1; i++) {
        if (!IsValidChar(StrGetChar(line, i))) SyntaxErrorInvalidChar(tc, i, "illegal character");
    }
}


void ParseLine(struct tokenContext* tc) {
    bool eof = false;
    struct str line = ReadLine(tc->fp, &eof);
    StrListAppend(&(tc->lines), line);
    ValidateLatestLine(tc);

    int col = 0;
    struct token tok;
    while(col < StrGetLen(line) && ParseToken(tc, &col, &tok)) {
        tokenPipePush(&(tc->tokens), tok);
    }
    if (eof) {
        tokenPipePush(&(tc->tokens), TokenEOF(tc));
    }
}


struct token TokenNext(struct tokenContext* tc) {
    while (tokenPipeIsEmpty(&(tc->tokens))) ParseLine(tc);
    struct token tok = tokenPipePop(&(tc->tokens));
    if (tok.type == TOKEN_EOF) tokenPipePush(&(tc->tokens), tok);
    return tok;
}


struct token TokenPeek(struct tokenContext* tc) {
    if (tokenPipeIsEmpty(&(tc->tokens))) ParseLine(tc);
    return tokenPipePeek(&(tc->tokens));
}


void TokenUnget(struct tokenContext* tc, int n) {
    tokenPipeUnpop(&(tc->tokens), n);
}


void TokenRestart(struct tokenContext* tc) {
    tokenPipeRestart(&(tc->tokens));
}


//TODO multiline tokens
struct token TokenExtend(struct token base, struct token tail) { //assumes the tokens exist on the same line
    if (base.lineNr != tail.lineNr) Error("base and tail tokens exist on different lines");
    struct token ret;
    ret.type = base.type;
    ret.lineNr = base.lineNr;
    ret.str = StrGetContainsBoth(base.str, tail.str);
    ret.context = base.context;
    return ret;
}


char* TokenTypeToString(enum tokenType t) {
    switch(t) {
        case TOKEN_UNDEF: return "";
        case TOKEN_EOF: return "end of file";
        case TOKEN_INT: return "integer literals";
        case TOKEN_FLOAT: return "float literals";
        case TOKEN_CHAR: return "character literals";
        case TOKEN_STRING: return "string literals";
        case TOKEN_IDENTIFIER: return "identifier";
        case TOKEN_IMPORT: return "import";
        case TOKEN_TYPE: return "type";;
        case TOKEN_IF: return "if";
        case TOKEN_ELSE: return "else";
        case TOKEN_FOR: return "for";
        case TOKEN_DEFER: return "defer";;
        case TOKEN_RETURN: return "return";
        case TOKEN_BREAK: return "break";
        case TOKEN_MATCH: return "break";
        case TOKEN_CASE: return "case";;
        case TOKEN_STRUCT: return "struct";
        case TOKEN_VOCAB: return "vocab";
        case TOKEN_FUNC: return "func";
        case TOKEN_DOT: return ".";;
        case TOKEN_COMMA: return ",";
        case TOKEN_PAREN_OPEN: return "(";
        case TOKEN_PAREN_CLOSE: return ")";
        case TOKEN_SQUAREBRACKET_OPEN: return "[";;
        case TOKEN_SQUAREBRACKET_CLOSE: return "]";
        case TOKEN_CURLYBRACKET_OPEN: return "{";
        case TOKEN_CURLYBRACKET_CLOSE: return "}";
        case TOKEN_ASSIGNMENT: return "=";
        case TOKEN_ASSIGNMENT_ADD: return "+=";
        case TOKEN_ASSIGNMENT_SUB: return "-=";
        case TOKEN_ASSIGNMENT_MUL: return "*=";
        case TOKEN_ASSIGNMENT_DIV: return "/=";
        case TOKEN_ASSIGNMENT_MODULO: return "%=";
        case TOKEN_LOGICAL_EQUALS: return "==";
        case TOKEN_LOGICAL_NOT_EQUALS: return "!=";
        case TOKEN_LOGICAL_NOT: return "==";
        case TOKEN_LOGICAL_AND: return "&&";
        case TOKEN_LOGICAL_OR: return "||";
        case TOKEN_LOGICAL_LESS_THAN: return "<";
        case TOKEN_LOGICAL_LESS_THAN_OR_EQUAL: return "<=";
        case TOKEN_LOGICAL_GREATER_THAN: return ">";
        case TOKEN_LOGICAL_GREATER_THAN_OR_EQUAL: return ">=";
        case TOKEN_BITWISE_AND: return "&";
        case TOKEN_BITWISE_OR: return "|";
        case TOKEN_BITWISE_XOR: return "^";
        case TOKEN_BITWISE_COMPLEMENT: return "~";
        case TOKEN_BITSHIFT_LEFT: return "<<";
        case TOKEN_BITSHIFT_RIGHT: return ">>";
        case TOKEN_ADD: return "+";
        case TOKEN_SUB: return "-";
        case TOKEN_MUL: return "*";
        case TOKEN_DIV: return "/";
        case TOKEN_MODULO: return "%";
        case TOKEN_INCREMENT: return "++";
        case TOKEN_DECREMENT: return "--";
        case TOKEN_MUT: return "mut";
    }
    Error("invalid token type");
    exit(EXIT_FAILURE);
}
