#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "parser.h"
#include "str.h"
#include "token.h"
#include "error.h"


void TypeListAppend(struct typeList* tl, struct type type) {
    if (tl->len >= tl->cap) {
        tl->cap += 100;
        tl->ptr = realloc(tl->ptr, sizeof(struct type) * tl->cap);
        CheckPtr(tl->ptr);
    }
    tl->ptr[tl->len] = type;
    tl->len++;
}


static struct type BaseType(char* name, enum baseType bType) {
    struct type t;
    t.bType = bType;
    t.name = StrFromCharArray(name);
    t.advanced = NULL;
    return t;
}


struct type ArrType(struct type heldType, int len) {
    struct arrTypeData* arrT = malloc(sizeof(struct arrTypeData));
    CheckPtr(arrT);
    arrT->heldType = heldType;
    arrT->len = len;

    struct type t;
    t.bType = BASETYPE_ARRAY;
    t.name = StrNew();
    t.advanced = arrT;
    return t;
}


struct type StructType(struct operandList members) {
    struct structTypeData* structT = malloc(sizeof(struct structTypeData));
    CheckPtr(structT);
    structT->members = members;

    struct type t;
    t.bType = BASETYPE_STRUCT;
    t.name = StrNew();
    t.advanced = structT;
    return t;
}


struct type VocabType(struct strList words) {
    struct vocabTypeData* vocabT = malloc(sizeof(struct vocabTypeData));
    CheckPtr(vocabT);
    vocabT->words = words;

    struct type t;
    t.bType = BASETYPE_VOCAB;
    t.name = StrNew();
    t.advanced = vocabT;
    return t;
}


struct type FuncType(struct typeList args, struct typeList rets) {
    struct funcTypeData* funcT = malloc(sizeof(struct vocabTypeData));
    CheckPtr(funcT);
    funcT->args = args;
    funcT->rets = rets;

    struct type t;
    t.bType = BASETYPE_FUNC;
    t.name = StrNew();
    t.advanced = funcT;
    return t;

}


struct type AliasType(struct type old, struct str name) {
    old.name = name;
    return old;
}


static void AppendBaseTypes(struct typeList* tl) {
    TypeListAppend(tl, BaseType("byte", BASETYPE_BYTE));
    TypeListAppend(tl, BaseType("bool", BASETYPE_BOOL));
    TypeListAppend(tl, BaseType("int8", BASETYPE_INT8));
    TypeListAppend(tl, BaseType("int16", BASETYPE_INT16));
    TypeListAppend(tl, BaseType("int32", BASETYPE_INT32));
    TypeListAppend(tl, BaseType("int64", BASETYPE_INT64));
    TypeListAppend(tl, BaseType("uint8", BASETYPE_UINT8));
    TypeListAppend(tl, BaseType("uint16", BASETYPE_UINT16));
    TypeListAppend(tl, BaseType("uint32", BASETYPE_UINT32));
    TypeListAppend(tl, BaseType("uint64", BASETYPE_UINT64));
    TypeListAppend(tl, BaseType("float32", BASETYPE_FLOAT32));
    TypeListAppend(tl, BaseType("float64", BASETYPE_FLOAT64));
}


struct typeList TypeListNew() {
    struct typeList tl;
    tl.len = 0;
    tl.cap = 0;
    tl.ptr = NULL;
    AppendBaseTypes(&tl);
    return tl;
}


static bool typeListGet(struct typeList tl, struct str name, struct type* t) {
    for (int i = 0; i < tl.len; i++) {
        if (StrEqual(tl.ptr[i].name, name)) {
            *t = tl.ptr[i];
            return true;
        }
    }
    return false;
}


bool GetType(struct str name, struct type* t, struct typeList publ, struct typeList priv) {
    if (typeListGet(publ, name, t)) return true;
    if (typeListGet(priv, name, t)) return true;
    return false;
}


//value may be NULL
struct operand OperandNew(struct str name, struct type type, char* value) {
    struct operand op;
    op.name = name;
    op.type = type;
    op.value = value;
    return op;
}


struct operandList OperandListNew() {
    struct operandList ol;
    ol.len = 0;
    ol.cap = 0;
    ol.ptr = NULL;
    return ol;
}


void operandListAppend(struct operandList* ol, struct operand op) {
    if (ol->len >= ol->cap) {
        ol->cap += 100;
        ol->ptr = realloc(ol->ptr, sizeof(struct operand) * ol->cap);
        CheckPtr(ol->ptr);
    }
    ol->ptr[ol->len] = op;
    ol->len++;
}


static bool operandListGet(struct operandList ol, struct str name, struct operand* op) {
    for (int i = 0; i < ol.len; i++) {
        if (StrEqual(ol.ptr[i].name, name)) {
            *op = ol.ptr[i];
            return true;
        }
    }
    return false;
}


bool GetOperand(struct str name, struct operand* op, struct operandList publ,
        struct operandList priv, struct operandList local) {
    if (operandListGet(publ, name, op)) return true;
    if (operandListGet(priv, name, op)) return true;
    if (operandListGet(local, name, op)) return true;
    return false;
}


static bool isPublic(struct str str) {
    if (StrGetLen(str) <= 0) Error("can not check publicity on empty string");
    char c = StrGetChar(str, 0);
    if (c >= 'A' && c <= 'Z') return true;
    return false;
}


static bool TryParseToken(struct parserContext* pc, struct token* tok,
        enum tokenType type, bool discardNewlines) {

    if (discardNewlines) {
        *tok = TokenNextDiscardNewlines(&(pc->tc));
    }
    else *tok = TokenNext(&(pc->tc));
    if (tok->type != type) {
        TokenUnget(&(pc->tc), *tok);
        return false;
    }
    return true;
}



static void ForceParseToken(struct parserContext* pc, struct token* tok,
        enum tokenType type, bool discardNewlines) {

    if (discardNewlines) {
        *tok = TokenNextDiscardNewlines(&(pc->tc));
    }
    else *tok = TokenNext(&(pc->tc));
    if (tok->type != type) {
        SyntaxErrorInvalidToken(*tok, NULL);
    }
}


struct type ParseNormalType(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, false);
    struct type t;
    if (!GetType(tok.str, &t, pc->publTypes, pc->privTypes)) SyntaxErrorInvalidToken(tok, "unknown type");
    tok = TokenNext(&(pc->tc));
    if (tok.type == TOKEN_SQUAREBRACKET_OPEN) {
        struct type arrT = ArrType(t, 0);
        ForceParseToken(pc, &tok, TOKEN_SQUAREBRACKET_CLOSE, false);
        return arrT;
    }
    return t;
}


static struct type ParseType(struct parserContext* pc);


struct operand ParseOperandTypeCombo(struct parserContext* pc) {
    struct token opName;
    ForceParseToken(pc, &opName, TOKEN_IDENTIFIER, false);
    struct type t = ParseType(pc);
    return OperandNew(opName.str, t, NULL);
}


struct operandList ParseOperandTypeComboSequence(struct parserContext* pc) {
    struct operandList list = OperandListNew();
    struct token tok;
    operandListAppend(&list, ParseOperandTypeCombo(pc));
    while (TryParseToken(pc, &tok, TOKEN_COMMA, true)) {
        TokenDiscardNewlines(&(pc->tc));
        operandListAppend(&list, ParseOperandTypeCombo(pc));
    }
    return list;
}


struct type ParseStructType(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, false);
    TokenDiscardNewlines(&(pc->tc));
    struct operandList members = ParseOperandTypeComboSequence(pc);
    ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, true);
    return StructType(members);
}


struct strList ParseIdentifierSequence(struct parserContext* pc) {
    struct strList list = StrListNew();
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, true);
    StrListAppend(&list, tok.str);
    while (TryParseToken(pc, &tok, TOKEN_COMMA, true)) {
        ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, true);
        StrListAppend(&list, tok.str);
    }
    return list;
}


struct type ParseVocabType(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, false);
    struct strList words = ParseIdentifierSequence(pc);
    ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, false);
    return VocabType(words);
}


struct typeList ParseTypeSequence(struct parserContext* pc) {
    struct typeList list = TypeListNew();
    TypeListAppend(&list, ParseType(pc));
    struct token tok;
    while (TryParseToken(pc, &tok, TOKEN_COMMA, true)) {
        TypeListAppend(&list, ParseType(pc));
    }
    return list;
}


struct type ParseFuncType(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_PAREN_OPEN, true);
    struct typeList args = ParseTypeSequence(pc);
    ForceParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true);
    ForceParseToken(pc, &tok, TOKEN_PAREN_OPEN, true);
    struct typeList rets = ParseTypeSequence(pc);
    ForceParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true);
    return FuncType(args, rets);
}


static struct type ParseType(struct parserContext* pc) {
    struct token tok = TokenNext(&(pc->tc));
    switch (tok.type) {
        case TOKEN_IDENTIFIER: TokenUnget(&(pc->tc), tok); return ParseNormalType(pc);
        case TOKEN_STRUCT: return ParseStructType(pc);
        case TOKEN_VOCAB: return ParseVocabType(pc);
        case TOKEN_FUNC: return ParseFuncType(pc);
        default: SyntaxErrorInvalidToken(tok, "not a type");
    }
    return (struct type){};
}


void ParseTypedef(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, false);
    struct type oldType = ParseType(pc);
    struct type newType = AliasType(oldType, tok.str);
    if (isPublic(tok.str)) TypeListAppend(&(pc->publTypes), newType);
    else TypeListAppend(&(pc->privTypes), newType);
    ForceParseToken(pc, &tok, TOKEN_NEWLINE, false);
}


void ParseGlobalLevel(struct parserContext* pc) {
    struct token tok = TokenNextDiscardNewlines(&(pc->tc));
    switch (tok.type) {
        case TOKEN_TYPE: ParseTypedef(pc); break;
        default: SyntaxErrorInvalidToken(tok, NULL); 
    }
}


struct parserContext parserContextNew(char* fileName) {
    struct parserContext pc;
    pc.tc = TokenContextNew(fileName);
    pc.publTypes = TypeListNew();
    pc.privTypes = TypeListNew();
    return pc;
}


void ParseFile(char* fileName) {
    struct parserContext pc = parserContextNew(fileName);
    struct token tok;
    while ((tok = TokenNextDiscardNewlines(&(pc.tc))).type != TOKEN_EOF) {
        TokenUnget(&(pc.tc), tok);
        ParseGlobalLevel(&pc);
    }
}
