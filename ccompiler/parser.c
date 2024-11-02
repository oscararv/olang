#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "str.h"
#include "token.h"
#include "error.h"


#define ARRAY_REF -1


//vanilla types must be initialized at startup
struct typeDef vanillaTypeTypeDef = {.complete = true};
struct type vanillaTypeBool;
struct type vanillaTypeBit8;
struct type vanillaTypeBit16;
struct type vanillaTypeBit32;
struct type vanillaTypeBit64;
struct type vanillaTypeInt8;
struct type vanillaTypeInt16;
struct type vanillaTypeInt32;
struct type vanillaTypeInt64;
struct type vanillaTypeFloat32;
struct type vanillaTypeFloat64;


struct typeList typeListNew() {
    return (struct typeList){0};
}


struct typePtrList typePtrListNew() {
    return (struct typePtrList){0};
}


struct opPtrList opPtrListNew() {
    return (struct opPtrList){0};
}


struct varList varListNew() {
    return (struct varList){0};
}


struct pcList pcListNew() {
    return (struct pcList){0};
}


void typeListAppend(struct typeList* tl, struct type t) {
    if (tl->len >= tl->cap) {
        tl->cap += 100;
        tl->ptr = realloc(tl->ptr, sizeof(*(tl->ptr)) * tl->cap);
        CheckPtr(tl->ptr);
    }
    tl->ptr[tl->len] = t;
    (tl->len)++;
}


void typePtrListAppend(struct typePtrList* tpl, struct type* t) {
    if (tpl->len >= tpl->cap) {
        tpl->cap += 100;
        tpl->ptr = realloc(tpl->ptr, sizeof(*(tpl->ptr)) * tpl->cap);
        CheckPtr(tpl->ptr);
    }
    tpl->ptr[tpl->len] = t;
    (tpl->len)++;
}


void opPtrListAppend(struct opPtrList* opl, struct operand* o) {
    if (opl->isSlice) Error("list slices may not be appended");
    if (opl->len >= opl->cap) {
        opl->cap += 100;
        opl->ptr = realloc(opl->ptr, sizeof(*(opl->ptr)) * opl->cap);
        CheckPtr(opl->ptr);
    }
    opl->ptr[opl->len] = o;
    (opl->len)++;
}


struct opPtrList opPtrListSlice(struct opPtrList orig, int start, int stop) {
    if (stop > orig.len) Error("slice stop index is greater than original string len");
    if (start < 0) Error("slice start index is less than zero");
    orig.isSlice = true;
    orig.len = stop - start;
    orig.ptr = orig.ptr + start;
    return orig;
}


void varListAppend(struct varList* vl, struct variable v) {
    if (vl->len >= vl->cap) {
        vl->cap += 100;
        vl->ptr = realloc(vl->ptr, sizeof(*(vl->ptr)) * vl->cap);
        CheckPtr(vl->ptr);
    }
    vl->ptr[vl->len] = v;
    (vl->len)++;
}


void pcListAppend(struct pcList* pcl, struct parserContext pc) {
    if (pcl->len >= pcl->cap) {
        pcl->cap += 100;
        pcl->ptr = realloc(pcl->ptr, sizeof(*(pcl->ptr)) * pcl->cap);
        CheckPtr(pcl->ptr);
    }
    pcl->ptr[pcl->len] = pc;
    (pcl->len)++;
}


struct type* isVanillaType(struct str name) {
    if (StrEqual(name, vanillaTypeBool.name)) return &vanillaTypeBool;
    if (StrEqual(name, vanillaTypeInt8.name)) return &vanillaTypeInt8;
    if (StrEqual(name, vanillaTypeInt16.name)) return &vanillaTypeInt16;
    if (StrEqual(name, vanillaTypeInt32.name)) return &vanillaTypeInt32;
    if (StrEqual(name, vanillaTypeInt64.name)) return &vanillaTypeInt64;
    if (StrEqual(name, vanillaTypeFloat32.name)) return &vanillaTypeFloat32;
    if (StrEqual(name, vanillaTypeFloat64.name)) return &vanillaTypeFloat64;
    return NULL;
}


bool typeListContains(struct typeList* tl, struct str name) {
    struct type* t = isVanillaType(name);
    if (t) return true;
    for (int i = 0; i < tl->len; i++) {
        if (StrEqual(tl->ptr[i].name, name)) return true;
    }
    return false;
}


bool typePtrListContains(struct typePtrList* tpl, struct str name) {
    struct type* t = isVanillaType(name);
    if (t) return true;
    for (int i = 0; i < tpl->len; i++) {
        if (StrEqual(tpl->ptr[i]->name, name)) return true;
    }
    return false;
}


bool typePtrListContainsPtr(struct typePtrList* tpl, struct type* ptr) {
    for (int i = 0; i < tpl->len; i++) {
        if (tpl->ptr[i] == ptr) return true;
    }
    return false;
}


bool varListContains(struct varList* vl, struct str name) {
    for (int i = 0; i < vl->len; i++) {
        if (StrEqual(vl->ptr[i].name, name)) return true;
    }
    return false;
}


bool pcListContains(struct pcList* pcl, struct str fileName) {
    for (int i = 0; i < pcl->len; i++) {
        if (StrEqual(pcl->ptr[i].fileName, fileName)) return true;
    }
    return false;
}


void typeListUpdate(struct typeList* tl, struct type t) {
    for (int i = 0; i < tl->len; i++) {
        if (StrEqual(tl->ptr[i].name, t.name)) {
            tl->ptr[i] = t;
            return;
        }
    }
    Error("type does not exist in type list");
    exit(EXIT_FAILURE); //unreachable
}


struct type* typeListGet(struct typeList* tl, struct str name) {
    struct type* t = isVanillaType(name);
    if (t) return t;
    for (int i = 0; i < tl->len; i++) {
        if (StrEqual(tl->ptr[i].name, name)) return tl->ptr + i;
    }
    Error("type does not exist in type list");
    exit(EXIT_FAILURE); //unreachable
}


struct operand* opPtrListGet(struct opPtrList* opl, int index) {
    if (index >= opl->len) {
        Error("index out of bounds");
        exit(EXIT_FAILURE); //unreachable
    }
    return opl->ptr[index];
}


struct variable* varListGet(struct varList* vl, struct str name) {
    for (int i = 0; i < vl->len; i++) {
        if (StrEqual(vl->ptr[i].name, name)) return vl->ptr + i;
    }
    Error("variable does not exist in variable list");
    exit(EXIT_FAILURE); //unreachable
}


struct parserContext* pcListGet(struct pcList* pcl, struct str fileName) {
    for (int i = 0; i < pcl->len; i++) {
        if (StrEqual(pcl->ptr[i].fileName, fileName)) return pcl->ptr + i;
    }
    Error("variable does not exist in variable list");
    exit(EXIT_FAILURE); //unreachable
}


bool isPublic(struct str str) {
    if (StrGetLen(str) <= 0) Error("can not check publicity on empty string");
    char c = StrGetChar(str, 0);
    if (c >= 'A' && c <= 'Z') return true;
    return false;
}


struct typeDef* typeDefEmpty() {
    struct typeDef* t = malloc(sizeof(*t));
    CheckPtr(t);
    *t = (struct typeDef){0};
    return t;
}


bool pcContainsType(struct parserContext* pc, struct str name) {
    if (isPublic(name) && typeListContains(&(pc->publTypes), name)) return true;
    return typeListContains(&(pc->privTypes), name);
}


bool pcContainsVar(struct parserContext* pc, struct str name) {
    if (isPublic(name) && varListContains(&(pc->publVars), name)) return true;
    return varListContains(&(pc->privVars), name);
}


bool pcContainsSymbol(struct parserContext* pc, struct str name) {
    if (pcContainsType(pc, name)) return true;
    return pcContainsVar(pc, name);
}


struct variable* pcGetVar(struct parserContext* pc, struct str name) {
    if (isPublic(name)) return varListGet(&(pc->publVars), name);
    return varListGet(&(pc->privVars), name);
}


struct type* pcGetTypePtr(struct parserContext* pc, struct str name) {
    if (isPublic(name)) return typeListGet(&(pc->publTypes), name);
    return typeListGet(&(pc->privTypes), name);
}


void updateType(struct parserContext* pc, struct type t) {
    if (isPublic(t.name)) typeListUpdate(&(pc->publTypes), t);
    else typeListUpdate(&(pc->privTypes), t);
}


void pcAddType(struct parserContext* pc, struct type t) {
    if (pcContainsSymbol(pc, t.name)) SyntaxErrorInvalidToken(t.tok, "symbol already defined");
    if (isPublic(t.name)) typeListAppend(&(pc->publTypes), t);
    else typeListAppend(&(pc->privTypes), t);
}


void pcAddVar(struct parserContext* pc, struct variable v) {
    if (pcContainsSymbol(pc, v.name)) SyntaxErrorInvalidToken(v.tok, "symbol already defined");
    if (isPublic(v.name)) varListAppend(&(pc->publVars), v);
    else varListAppend(&(pc->privVars), v);
}


struct typeList getEmbeddedStructs(struct varList* members) {
    struct typeList embedded = typeListNew();
    for (int i = 0; i < members->len; i++) {
        struct type membType = members->ptr[i].type;
        if (membType.bType == BASETYPE_STRUCT && !(membType.ref)) {
            typeListAppend(&embedded, membType);
            if (membType.def->complete) {
                struct typeList contained = getEmbeddedStructs(&(membType.def->members));
                for (int i = 0; i < contained.len; i++) {
                    typeListAppend(&embedded, contained.ptr[i]);
                }
            }
        }
    }
    return embedded;
}


struct type placeholderType(struct token nameTok) {
    struct type t = {0};
    t.name = nameTok.str;
    t.tok = nameTok;
    t.bType = BASETYPE_PLACEHOLDER;
    t.def = typeDefEmpty();
    t.ref = true;
    return t;
}


struct type importType() {
    struct typeDef* tDef = typeDefEmpty();
    tDef->complete = true;

    struct type t = {0};
    t.name = StrNew();
    t.tok = TOKEN_UNDEFINED;
    t.bType = BASETYPE_IMPORT;
    t.def = tDef;
    return t;
}


struct type structType(struct token nameTok, struct varList members) {
    struct typeDef* tDef = typeDefEmpty();
    tDef->complete = true;
    tDef->members = members;
    tDef->embeddedStructs = getEmbeddedStructs(&members);

    struct type t = {0};
    t.name = nameTok.str;
    t.tok = nameTok;
    t.bType = BASETYPE_STRUCT;
    t.def = tDef;
    t.ref = true;
    return t;
}


struct type vocabType(struct token nameTok, struct strList words) {
    struct typeDef* tDef = typeDefEmpty();
    tDef->complete = true;
    tDef->words = words;

    struct type t = {0};
    t.name = nameTok.str;
    t.tok = nameTok;
    t.bType = BASETYPE_VOCAB;
    t.def = tDef;
    return t;
}


struct type funcType(struct token nameTok, struct varList args, struct typeList rets) {
    struct typeDef* tDef = typeDefEmpty();
    tDef->complete = true;
    tDef->args = args;
    tDef->rets = rets;

    struct type t = {0};
    t.name = nameTok.str;
    t.tok = nameTok;
    t.bType = BASETYPE_FUNC;
    t.def = tDef;
    return t;
}


struct type aliasType(struct token nameTok, struct type oldNameT, struct typePtrList dependants) {
    oldNameT.name = nameTok.str;
    oldNameT.tok = nameTok;
    oldNameT.dependants = dependants;
    return oldNameT;
}


bool tryParseToken(struct parserContext* pc, struct token* tok, enum tokenType type) {
    struct token tmpTok = TokenNext(pc->tc);
    if (tmpTok.type != type) {
        TokenUnget(pc->tc, 1);
        return false;
    }
    *tok = tmpTok;
    return true;
}


//errorMsg is autogenerated on NULL
void parseToken(struct parserContext* pc, struct token* tok, enum tokenType type, char* errorMsg) {
    if (!tryParseToken(pc, tok, type)) {
        if (errorMsg == NULL) {
            char msg[100];
            strcpy(msg, "expected ");
            strcat(msg, TokenTypeToString(type));
            errorMsg = msg;
        }
        SyntaxErrorInvalidToken(TokenPeek(pc->tc), errorMsg);
    }
}


struct operand* operandEmpty() {
    struct operand* op = malloc(sizeof(*op));
    CheckPtr(op);
    *op = (struct operand){0};
    op->tok = TOKEN_UNDEFINED;
    return op;
}


struct operand* operandIntLiteralNoToken(long long intVal) {
    struct operand* op = operandEmpty();
    op->tok = TOKEN_UNDEFINED;
    op->type = vanillaTypeInt64;
    op->type.constLiteral = true;
    op->valKnown = true;
    op->intVal = intVal;
    op->operation = OPERATION_NOOP;
    return op;

}


struct operand* operandIntLiteral(struct token valueTok) {
    struct operand* op = operandIntLiteralNoToken(StrToLongLong(valueTok.str));
    op->tok = valueTok;
    return op;
}


struct operand* operandFloatLiteral(struct token valueTok) {
    struct operand* op = operandEmpty();
    op->tok = valueTok;
    op->type = vanillaTypeFloat64;
    op->type.constLiteral = true;
    op->valKnown = true;
    op->floatVal = StrToDouble(valueTok.str);
    op->operation = OPERATION_NOOP;
    return op;
}


struct operand* operandCharLiteral(struct token valueTok) {
    struct operand* op = operandEmpty();
    op->tok = valueTok;
    op->type = vanillaTypeInt8;
    op->type.constLiteral = true;
    op->valKnown = true;
    op->intVal = StrToChar(valueTok.str);
    op->operation = OPERATION_NOOP;
    return op;
}


struct type stringType(struct token tok) {
    struct type t = vanillaTypeBit8;
    t.tok = tok;
    t.bType = BASETYPE_ARRAY;
    t.arrBType = BASETYPE_BIT8;
    return t;
}


struct operand* operandStringLiteral(struct token valueTok) {
    struct operand* op = operandEmpty();
    op->tok = valueTok;
    op->type = stringType(valueTok);
    op->type.constLiteral = true;
    op->valKnown = true;
    op->arrVal = StrToString(valueTok.str);
    op->operation = OPERATION_NOOP;
    return op;
}


bool isBit(struct type t) {
    if (t.bType == BASETYPE_BIT8) return true;
    if (t.bType == BASETYPE_BIT16) return true;
    if (t.bType == BASETYPE_BIT32) return true;
    if (t.bType == BASETYPE_BIT64) return true;
    return false;
}



bool isInt(struct type t) {
    if (t.bType == BASETYPE_INT8) return true;
    if (t.bType == BASETYPE_INT16) return true;
    if (t.bType == BASETYPE_INT32) return true;
    if (t.bType == BASETYPE_INT64) return true;
    return false;
}


bool isFloat(struct type t) {
    if (t.bType == BASETYPE_FLOAT32) return true;
    if (t.bType == BASETYPE_FLOAT64) return true;
    return false;
}


bool isNumber(struct type t) {
    if (isInt(t)) return true;
    if (isFloat(t)) return true;
    return false;
}


bool isCastable(struct operand* from, struct type to) {
    if (isNumber(to) && isNumber(from->type)) return true;
    if (to.bType == BASETYPE_ARRAY && to.arrBType == BASETYPE_BIT8) return true;
    if (from->type.bType == BASETYPE_ARRAY && from->type.arrBType == BASETYPE_BIT8) return true;
    return false;
}


bool tryParseUnaryTok(struct parserContext* pc, struct token* tok) {
    struct token tmpTok = TokenNext(pc->tc);
    switch (tmpTok.type) {
        case TOKEN_ADD: *tok = tmpTok; return true;
        case TOKEN_SUB: *tok = tmpTok; return true;
        case TOKEN_LOGICAL_NOT: *tok = tmpTok; return true;
        case TOKEN_BITWISE_COMPLEMENT: *tok = tmpTok; return true;
        default: TokenUnget(pc->tc, 1); return false;
    }
}


bool tryParseBinaryTok(struct parserContext* pc, struct token* tok) {
    struct token tmpTok = TokenNext(pc->tc);
    switch (tmpTok.type) {
        case TOKEN_MUL: *tok = tmpTok; return true;
        case TOKEN_DIV: *tok = tmpTok; return true;
        case TOKEN_ADD: *tok = tmpTok; return true;
        case TOKEN_SUB: *tok = tmpTok; return true;
        case TOKEN_LOGICAL_EQUALS: *tok = tmpTok; return true;
        case TOKEN_LOGICAL_AND: *tok = tmpTok; return true;
        case TOKEN_LOGICAL_OR: *tok = tmpTok; return true;
        case TOKEN_LOGICAL_LESS_THAN: *tok = tmpTok; return true;
        case TOKEN_LOGICAL_LESS_THAN_OR_EQUAL: *tok = tmpTok; return true;
        case TOKEN_LOGICAL_GREATER_THAN: *tok = tmpTok; return true;
        case TOKEN_LOGICAL_GREATER_THAN_OR_EQUAL: *tok = tmpTok; return true;
        case TOKEN_MODULO: *tok = tmpTok; return true;
        case TOKEN_BITSHIFT_LEFT: *tok = tmpTok; return true;
        case TOKEN_BITSHIFT_RIGHT: *tok = tmpTok; return true;
        case TOKEN_BITWISE_AND: *tok = tmpTok; return true;
        case TOKEN_BITWISE_OR: *tok = tmpTok; return true;
        case TOKEN_BITWISE_XOR: *tok = tmpTok; return true;
        default: TokenUnget(pc->tc, 1); return false;
    }
}


enum operationType operationFromTok(struct token tok) {
    switch (tok.type) {
        case TOKEN_LOGICAL_NOT: return OPERATION_LOGICAL_NOT;
        case TOKEN_BITWISE_COMPLEMENT: return OPERATION_BITWISE_COMPLEMENT;
        case TOKEN_MUL: return OPERATION_MUL;
        case TOKEN_DIV: return OPERATION_DIV;
        case TOKEN_ADD: return OPERATION_ADD;
        case TOKEN_SUB: return OPERATION_SUB;
        case TOKEN_LOGICAL_EQUALS: return OPERATION_LOGICAL_EQUALS;
        case TOKEN_LOGICAL_NOT_EQUALS: return OPERATION_LOGICAL_NOT_EQUALS;
        case TOKEN_LOGICAL_AND: return OPERATION_LOGICAL_AND;
        case TOKEN_LOGICAL_OR: return OPERATION_LOGICAL_OR;
        case TOKEN_LOGICAL_LESS_THAN: return OPERATION_LOGICAL_LESS_THAN;
        case TOKEN_LOGICAL_LESS_THAN_OR_EQUAL: return OPERATION_LOGICAL_LESS_THAN_OR_EQUAL;
        case TOKEN_LOGICAL_GREATER_THAN: return OPERATION_LOGICAL_GREATER_THAN;
        case TOKEN_LOGICAL_GREATER_THAN_OR_EQUAL: return OPERATION_LOGICAL_GREATER_THAN_OR_EQUAL;
        case TOKEN_MODULO: return OPERATION_MODULO;
        case TOKEN_BITSHIFT_LEFT: return OPERATION_BITSHIFT_LEFT;
        case TOKEN_BITSHIFT_RIGHT: return OPERATION_BITSHIFT_RIGHT;
        case TOKEN_BITWISE_AND: return OPERATION_BITWISE_AND;
        case TOKEN_BITWISE_OR: return OPERATION_BITWISE_OR;
        case TOKEN_BITWISE_XOR: return OPERATION_BITWISE_XOR;
        case TOKEN_INCREMENT: return OPERATION_INCREMENT;
        case TOKEN_DECREMENT: return OPERATION_DECREMENT;
        case TOKEN_ASSIGNMENT: return OPERATION_ASSIGNMENT;
        case TOKEN_ASSIGNMENT_ADD: return OPERATION_ASSIGNMENT_ADD;
        case TOKEN_ASSIGNMENT_SUB: return OPERATION_ASSIGNMENT_SUB;
        case TOKEN_ASSIGNMENT_MUL: return OPERATION_ASSIGNMENT_MUL;
        case TOKEN_ASSIGNMENT_DIV: return OPERATION_ASSIGNMENT_DIV;
        case TOKEN_ASSIGNMENT_MODULO: return OPERATION_ASSIGNMENT_MODULO;
        default: exit(EXIT_FAILURE); //programming error
    }
}


void evalUnary(struct operand* op) {
    if (!(op->operands.ptr[0]->valKnown)) return;
    switch(op->operation) {
        case OPERATION_ADD: break;
        case OPERATION_SUB: if (isInt(op->type)) op->intVal = -op->intVal; else op->floatVal = -op->floatVal; break;
        case OPERATION_LOGICAL_NOT: if (op->intVal) op->intVal = 0; else op->intVal = 1; break;
        case OPERATION_BITWISE_COMPLEMENT: op->intVal = ~(op->intVal); break;
        default: break;
    }
    op->valKnown = true;
}


struct operand* unaryOperand(struct token unaryTok, struct operand* from) {
    enum operationType oper = operationFromTok(unaryTok);
    if (oper == OPERATION_ADD && !isNumber(from->type)) {
        SyntaxErrorInvalidToken(from->tok, "addition only defined for numbers");
        return NULL;
    }
    else if (oper == OPERATION_SUB && !isNumber(from->type)) {
        SyntaxErrorInvalidToken(from->tok, "subraction only defined for numbers");
        return NULL;
    }
    else if (oper == OPERATION_LOGICAL_NOT && from->type.bType != BASETYPE_BOOL) {
        SyntaxErrorInvalidToken(from->tok, "logical not operand only defined for bool");
        return NULL;
    }
    else if (!isInt(from->type)) {
        SyntaxErrorInvalidToken(from->tok, "bitwise complement only defined for integers");
        return NULL;
    }
    struct operand* to = operandEmpty();
    *to = *from;
    to->tok = TokenMerge(unaryTok, from->tok);
    to->operands = opPtrListNew();
    to->operation = oper;
    opPtrListAppend(&(to->operands), from);
    evalUnary(to);
    return to;
}


struct operand* tryParseExpr(struct parserContext* pc, struct varList* localVars);
bool tryParseVar(struct parserContext* pc, struct variable* v);
bool tryParseType(struct parserContext* pc, struct type* t);


void evalTypeCast(struct operand* op) {
    struct operand* prev = op->operands.ptr[0];
    if (!prev->valKnown) return;
    if (isInt(op->type)) {
        if (isFloat(prev->type)) op->intVal = (long long)(prev->floatVal);
        else op->intVal = prev->intVal;
    }
    else if (isFloat(op->type)) {
        if (isInt(prev->type)) op->floatVal = (double)(prev->intVal);
        else op->floatVal = prev->floatVal;
    }
    else op->arrVal = prev->arrVal;
    op->valKnown = true;
}


struct operand* tryParseTypeCastOperand(struct parserContext* pc, struct varList* localVars) {
    struct type t;
    if (!tryParseType(pc, &t)) return NULL;
    struct token tok;
    parseToken(pc, &tok, TOKEN_PAREN_OPEN, NULL);
    struct operand* from = tryParseExpr(pc, localVars);
    if (!from) {
        SyntaxErrorInvalidToken(TokenPeek(pc->tc), "expected expression");
        return NULL;
    }
    parseToken(pc, &tok, TOKEN_PAREN_CLOSE, NULL);

    if (!isCastable(from, t)) {
        SyntaxErrorInvalidToken(from->tok, "expression has invalid type for cast");
        return NULL;
    }
    struct operand* to = operandEmpty();
    to->tok = TokenMerge(t.tok, tok);
    to->type = t;
    opPtrListAppend(&(to->operands), from);
    to->operation = OPERATION_TYPECAST;
    evalTypeCast(to);
    return to;
}


int typeGetNumNBits(struct type t) {
    switch (t.bType) {
        case BASETYPE_INT8: return 8;
        case BASETYPE_INT16: return 16;
        case BASETYPE_INT32: return 32;
        case BASETYPE_INT64: return 64;
        case BASETYPE_FLOAT32: return 32;
        case BASETYPE_FLOAT64: return 64;
        default: exit(EXIT_FAILURE); //programming error
    }
}


struct type typeNumMostBits(struct type a, struct type b) {
    if (typeGetNumNBits(a) >= typeGetNumNBits(b)) return a;
    return b;
}


bool isConstLiteral(struct type t) {
    return t.constLiteral;
}


void binOpPromoteConstLiteralsSharedType(struct type* a, struct type* b) {
    if (!isNumber(*a) || !isNumber(*b)) return;
    if (isConstLiteral(*a) && isConstLiteral(*b)) {
        if (isFloat(*a) && !isFloat(*b)) *b = *a;
        else if (isFloat(*b) && !isFloat(*a)) *a = *b;
        struct type shared = typeNumMostBits(*a, *b);
        *a = shared;
        *b = shared;
    }
    else if (isConstLiteral(*a) && isNumber(*b)) {
        if (isFloat(*a) && isInt(*b));
        else *a = *b;
    }
    else if (isConstLiteral(*b) && isNumber(*a)) {
        if (isFloat(*b) && isInt(*a));
        else *b = *a;
    }
}


//may promote constant literals
bool typeEquivalent(struct type* a, struct type* b) {
    binOpPromoteConstLiteralsSharedType(a, b);
    if (a->bType != b->bType) return false;
    if (a->bType == BASETYPE_ARRAY && (a->arrBType != b->arrBType)) return false;
    if ((!isNumber(*a) || !isNumber(*b)) && (a->def != b->def)) return false;
    return true;
}


struct opPtrList tryParseExprList(struct parserContext* pc, struct varList* localVars) {
    struct opPtrList exprs = opPtrListNew();
    struct operand* op = tryParseExpr(pc, localVars);
    if (!op) return exprs;
    opPtrListAppend(&exprs, op);

    struct token tok;
    while (tryParseToken(pc, &tok, TOKEN_COMMA)) {
        op = tryParseExpr(pc, localVars);
        if (!op) SyntaxErrorInvalidToken(TokenPeek(pc->tc), "expected expression");
        opPtrListAppend(&exprs, op);
    }
    return exprs;
}


struct operand* parseFuncCall(struct parserContext* pc, struct variable funcVar, struct varList* localVars) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_PAREN_OPEN, NULL);
    struct opPtrList args = tryParseExprList(pc, localVars);
    parseToken(pc, &tok, TOKEN_PAREN_CLOSE, NULL);
    struct varList expected = funcVar.type.def->args;
    if (expected.len != args.len) SyntaxErrorInvalidToken(tok, "invalid number of arguments");
    for (int i = 0; i < expected.len; i++) {
        if (!typeEquivalent(&(expected.ptr[i].type), &(args.ptr[i]->type))) {
            SyntaxErrorInvalidToken(args.ptr[i]->tok, "expression has invalid type");
        }
    }

    struct operand* op = operandEmpty();
    op->tok = TokenMerge(funcVar.tok, tok);
    op->operands = args;
    op->operation = OPERATION_FUNCCALL;
    return op;
}


struct operand* tryParseOperand(struct parserContext* pc, struct varList* localVars) {
    struct token tok = TokenNext(pc->tc);
    if (varListContains(localVars, tok.str)) return varListGet(localVars, tok.str)->value;
    else if (tok.type == TOKEN_IDENTIFIER) {
        TokenUnget(pc->tc, 1);
        struct variable v;
        if (tryParseVar(pc, &v)) return v.value;
        else return NULL;
    }
    else if (tok.type == TOKEN_INT) return operandIntLiteral(tok);
    else if (tok.type == TOKEN_FLOAT) return operandFloatLiteral(tok);
    else if (tok.type == TOKEN_CHAR) return operandCharLiteral(tok);
    else if (tok.type == TOKEN_STRING) return operandStringLiteral(tok);
    TokenUnget(pc->tc, 1);
    return NULL;
}


struct operand* tryParseExprOperand(struct parserContext* pc, struct varList* localVars) {
    bool unary = false;
    struct token unaryTok;
    struct token parenTok;
    struct operand* op = NULL;

    if (tryParseUnaryTok(pc, &unaryTok)) unary = true;
    if ((op = tryParseOperand(pc, localVars)));
    else if ((op = tryParseTypeCastOperand(pc, localVars)));
    else if (tryParseToken(pc, &parenTok, TOKEN_PAREN_OPEN)) {
        op = tryParseExpr(pc, localVars);
        parseToken(pc, &parenTok, TOKEN_PAREN_CLOSE, NULL);
    }
    else if (unary) TokenUnget(pc->tc, 1);
    if (!op) return NULL;
    if (unary) return unaryOperand(unaryTok, op);
    return op;
}


enum tokenType binTokenEvalOrderBackwards[] = {
    TOKEN_LOGICAL_OR,
    TOKEN_LOGICAL_AND,
    TOKEN_BITWISE_OR,
    TOKEN_BITWISE_XOR,
    TOKEN_BITWISE_AND,
    TOKEN_LOGICAL_NOT_EQUALS,
    TOKEN_LOGICAL_EQUALS,
    TOKEN_LOGICAL_GREATER_THAN,
    TOKEN_LOGICAL_GREATER_THAN_OR_EQUAL,
    TOKEN_LOGICAL_LESS_THAN,
    TOKEN_LOGICAL_LESS_THAN_OR_EQUAL,
    TOKEN_BITSHIFT_LEFT,
    TOKEN_BITSHIFT_RIGHT,
    TOKEN_SUB,
    TOKEN_ADD,
    TOKEN_DIV,
    TOKEN_MUL,
    TOKEN_MODULO
};


bool binOpMergeArithmetic(struct type a, struct type b, struct type* resType) {
    if (!isNumber(a) || !isNumber(b)) return false;
    if (!typeEquivalent(&a, &b)) return false;
    *resType = a;
    return true;
}


bool binOpMergeUniversalLogical(struct type a, struct type b, struct type* resType) {
    if (!typeEquivalent(&a, &b)) return false;
    *resType = vanillaTypeBool;
    return true;
}


bool binOpMergeBooleanLogical(struct type a, struct type b, struct type* resType) {
    if (a.bType != BASETYPE_BOOL || b.bType != BASETYPE_BOOL) return false;
    *resType = a;
    return true;
}


bool binOpMergeNumericLogical(struct type a, struct type b, struct type* resType) {
    if (!isNumber(a) || !isNumber(b)) return false;
    if (!typeEquivalent(&a, &b)) return false;
    *resType = vanillaTypeBool;
    return true;
}


bool binOpMergeIntShift(struct type a, struct type b, struct type* resType) {
    if (!(isInt(a) || isBit(a))) return false;
    if (!isInt(b)) return false;
    *resType = a;
    return true;
}


bool binOpMergeBitwise(struct type a, struct type b, struct type* resType) {
    if (!isBit(a) || !isBit(b)) return false;
    if (!typeEquivalent(&a, &b)) return false;
    *resType = a;
    return true;
}


bool isArithmetic(enum operationType oper) {
    if (oper == OPERATION_MUL) return true;
    if (oper == OPERATION_DIV) return true;
    if (oper == OPERATION_ADD) return true;
    if (oper == OPERATION_SUB) return true;
    return false;
}


bool isBinUniversalLogical(enum operationType oper) {
    if (oper == OPERATION_LOGICAL_EQUALS) return true;
    if (oper == OPERATION_LOGICAL_NOT_EQUALS) return true;
    return false;
}


bool isBinBooleanLogical(enum operationType oper) {
    if (oper == OPERATION_LOGICAL_AND) return true;
    if (oper == OPERATION_LOGICAL_OR) return true;
    return false;
}


bool isNumericLogical(enum operationType oper) {
    if (oper == OPERATION_LOGICAL_LESS_THAN) return true;
    if (oper == OPERATION_LOGICAL_LESS_THAN_OR_EQUAL) return true;
    if (oper == OPERATION_LOGICAL_GREATER_THAN) return true;
    if (oper == OPERATION_LOGICAL_GREATER_THAN_OR_EQUAL) return true;
    return false;
}


bool isIntShift(enum operationType oper) {
    if (oper == OPERATION_MODULO) return true;
    if (oper == OPERATION_BITSHIFT_LEFT) return true;
    if (oper == OPERATION_BITSHIFT_RIGHT) return true;
    return false;
}


bool isBinBitwise(enum operationType oper) {
    if (oper == OPERATION_BITWISE_AND) return true;
    if (oper == OPERATION_BITWISE_OR) return true;
    if (oper == OPERATION_BITWISE_XOR) return true;
    return false;
}


bool binOpMergeType(enum operationType opType, struct type a, struct type b, struct type* resType) {
    if (isArithmetic(opType)) return binOpMergeArithmetic(a, b, resType);
    if (isBinUniversalLogical(opType)) return binOpMergeUniversalLogical(a, b, resType);
    if (isBinBooleanLogical(opType)) return binOpMergeBooleanLogical(a, b, resType);
    if (isNumericLogical(opType)) return binOpMergeNumericLogical(a, b, resType);
    if (isIntShift(opType)) return binOpMergeIntShift(a, b, resType);
    if (isBinBitwise(opType)) return binOpMergeBitwise(a, b, resType);
    return false;
}


long long getAsInt(struct operand* op) {
    if (isFloat(op->type)) return (long long)op->floatVal;
    else return op->intVal;
}


double getAsFloat(struct operand* op) {
    if (isInt(op->type)) return (double)op->intVal;
    else return op->floatVal;
}


void evalBinary(struct operand* op) {
    if (!(op->operands.ptr[0]->valKnown)) return;
    if (!(op->operands.ptr[1]->valKnown)) return;
    struct operand* opA = op->operands.ptr[0];
    struct operand* opB = op->operands.ptr[1];
    switch(op->operation) {
        case OPERATION_MUL:
            if (isInt(op->type)) op->intVal = getAsInt(opA) * getAsInt(opB);
            else op->floatVal = getAsFloat(opA) * getAsFloat(opB);
            break;

        case OPERATION_DIV:
            if (isInt(op->type)) op->intVal = getAsInt(opA) / getAsInt(opB);
            else op->floatVal = getAsFloat(opA) / getAsFloat(opB);
            break;

        case OPERATION_ADD:
            if (isInt(op->type)) op->intVal = getAsInt(opA) + getAsInt(opB);
            else op->floatVal = getAsFloat(opA) + getAsFloat(opB);
            break;

        case OPERATION_SUB:
            if (isInt(op->type)) op->intVal = getAsInt(opA) - getAsInt(opB);
            else op->floatVal = getAsFloat(opA) - getAsFloat(opB);
            break;

        case OPERATION_LOGICAL_EQUALS:
            //TODO
            break;
        case OPERATION_LOGICAL_NOT_EQUALS:
            //TODO
            break;

        case OPERATION_LOGICAL_AND:
            op->intVal = opA->intVal && opB->intVal;
            break;

        case OPERATION_LOGICAL_OR:
            op->intVal = opA->intVal || opB->intVal;
            break;

        case OPERATION_LOGICAL_LESS_THAN:
            if (isInt(op->type)) op->intVal = getAsInt(opA) < getAsInt(opB);
            else op->intVal = getAsFloat(opA) < getAsFloat(opB);
            break;

        case OPERATION_LOGICAL_LESS_THAN_OR_EQUAL:
            if (isInt(op->type)) op->intVal = getAsInt(opA) <= getAsInt(opB);
            else op->intVal = getAsFloat(opA) <= getAsFloat(opB);
            break;

        case OPERATION_LOGICAL_GREATER_THAN:
            if (isInt(op->type)) op->intVal = getAsInt(opA) > getAsInt(opB);
            else op->intVal = getAsFloat(opA) > getAsFloat(opB);
            break;

        case OPERATION_LOGICAL_GREATER_THAN_OR_EQUAL:
            if (isInt(op->type)) op->intVal = getAsInt(opA) >= getAsInt(opB);
            else op->intVal = getAsFloat(opA) >= getAsFloat(opB);
            break;

        case OPERATION_MODULO:
            op->intVal = opA->intVal % opB->intVal;
            break;

        case OPERATION_BITSHIFT_LEFT:
            op->intVal = opA->intVal << opB->intVal;
            break;

        case OPERATION_BITSHIFT_RIGHT:
            op->intVal = opA->intVal >> opB->intVal;
            break;

        case OPERATION_BITWISE_AND:
            op->intVal = opA->intVal & opB->intVal;
            break;

        case OPERATION_BITWISE_OR:
            op->intVal = opA->intVal | opB->intVal;
            break;

        case OPERATION_BITWISE_XOR:
            op->intVal = opA->intVal ^ opB->intVal;
            break;

        default: break;
    }
    op->valKnown = true;
}


struct operand* binaryOperand(struct token binOpTok, struct operand* opA, struct operand* opB) {
    struct operand* op = operandEmpty();
    op->tok = TokenMerge(opA->tok, opB->tok);
    if (!binOpMergeType(operationFromTok(binOpTok), opA->type, opB->type, &(op->type))) {
        SyntaxErrorInvalidToken(op->tok, "incompatible type");
    }
    opPtrListAppend(&(op->operands), opA);
    opPtrListAppend(&(op->operands), opB);
    op->operation = operationFromTok(binOpTok);
    evalBinary(op);
    return op;
}


struct operand* evalExpr(struct opPtrList operands, struct tokenList tl) {
    for (int i = 0; i < (int)(sizeof(binTokenEvalOrderBackwards) / sizeof(enum tokenType)); i++) {
        for (int j = 0; j < tl.len; j++) {
            if (tl.ptr[j].type == binTokenEvalOrderBackwards[i]) {
                struct operand* opA = evalExpr(opPtrListSlice(operands, 0, j +1), TokenListSlice(tl, 0, j));
                struct operand* opB = evalExpr(opPtrListSlice(operands, j +1, operands.len),
                        TokenListSlice(tl, j +1, tl.len));
                return binaryOperand(tl.ptr[j], opA, opB);
            }
        }
    }
    return operands.ptr[0];
}


struct operand* tryParseExpr(struct parserContext* pc, struct varList* localVars) {
    struct opPtrList operands = opPtrListNew();
    struct tokenList binOps = TokenListNew();
    struct operand* op = tryParseExprOperand(pc, localVars);
    if (!op) return NULL;
    opPtrListAppend(&operands, op);

    struct token binaryTok;
    while (tryParseBinaryTok(pc, &binaryTok)) {
        TokenListAppend(&binOps, binaryTok);
        if (!(op = tryParseExprOperand(pc, localVars))) {
            SyntaxErrorInvalidToken(TokenPeek(pc->tc), "incomplete expression");
        }
        opPtrListAppend(&operands, op);
    }
    if (operands.len == 0) return NULL;
    return evalExpr(operands, binOps);
}


struct parserContext* getImportPc(struct parserContext* head, struct str alias) {
    struct variable* aliasVar = pcGetVar(head, alias);
    return pcListGet(head->parsedFiles, aliasVar->value->arrVal);
}


bool tryParseImportAlias(struct parserContext* pc, struct parserContext** import, struct token* aliasTok) {
    struct token tok;
    if (!tryParseToken(pc, &tok, TOKEN_IDENTIFIER)) return false;
    if (!pcContainsVar(pc, tok.str)) {
        TokenUnget(pc->tc, 1);
        return false;
    }
    *aliasTok = tok;
    *import = getImportPc(pc, tok.str);
    return true;
}


struct type* tryParseTypeIdentifier(struct parserContext* pc, struct str* name, struct token* typeToken) {
    bool import;
    struct token tok;
    struct parserContext* typeSource = pc;
    if ((import = tryParseImportAlias(pc, &typeSource, typeToken))) parseToken(pc, &tok, TOKEN_DOT, NULL);
    if (!tryParseToken(pc, &tok, TOKEN_IDENTIFIER)) {
        if (import) TokenUnget(pc->tc, 2);
        return NULL;
    }
    *name = tok.str;
    if (import) *typeToken = TokenMerge(*typeToken, tok);
    else *typeToken = tok;

    if (typeListContains(&(typeSource->publTypes), tok.str)) {
        return typeListGet(&(typeSource->publTypes), tok.str);
    }
    else if (!import && typeListContains(&(typeSource->privTypes), tok.str)) {
        return typeListGet(&(typeSource->privTypes), tok.str);
    }
    if (import) TokenUnget(pc->tc, 3);
    else TokenUnget(pc->tc, 1);
    return NULL;
}


void parseTypeIsStructInstantiation(struct parserContext* pc, struct type* t) {
    struct token tok;
    if (t->bType == BASETYPE_STRUCT && tryParseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN)) {
        parseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, NULL);
        t->tok = TokenMerge(t->tok, tok);
        t->ref = false;
    }
}


void assertIsValidArrTypeLen(struct operand* op) {
    if (!op->valKnown) SyntaxErrorInvalidToken(op->tok, "value must be computable at compile time");
    if (!isInt(op->type)) SyntaxErrorInvalidToken(op->tok, "array length must be an integer");
    if (op->intVal <= 0) SyntaxErrorInvalidToken(op->tok, "arrays cannot have negative or zero length");
}


void parseTypeIsArray(struct parserContext* pc, struct type* t) {
    struct token tok;
    bool isArray = false;
    while (tryParseToken(pc, &tok, TOKEN_SQUAREBRACKET_OPEN)) {
        isArray = true;
        struct operand* arrLen;
        struct varList vl = varListNew();
        if ((arrLen = tryParseExpr(pc, &vl))) assertIsValidArrTypeLen(arrLen);
        else arrLen = operandIntLiteralNoToken(ARRAY_REF);
        parseToken(pc, &tok, TOKEN_SQUAREBRACKET_CLOSE, NULL);
        t->tok = TokenMerge(t->tok, tok);
        opPtrListAppend(&(t->arrLenghts), arrLen);
    }
    if (isArray) {
        t->arrBType = t->bType;
        t->bType = BASETYPE_ARRAY;
        if (t->arrLenghts.ptr[0]->intVal != ARRAY_REF) t->ref = false;
    }
}


bool tryParseType(struct parserContext* pc, struct type* t) {
    struct token tok;
    struct str name;
    struct type* tPtr = tryParseTypeIdentifier(pc, &name, &tok);
    if (!tPtr) return false;
    *t = *tPtr;
    t->name = name;
    t->tok = tok;
    parseTypeIsStructInstantiation(pc, t);
    parseTypeIsArray(pc, t);
    return true;
}


struct type parseType(struct parserContext* pc) {
    struct type t;
    if (tryParseType(pc, &t)) return t;
    SyntaxErrorInvalidToken(TokenNext(pc->tc), "expected type");
    return t;
}


struct variable* tryParseVarIdentifier(struct parserContext* pc, struct str* name, struct token* varTok) {
    bool import;
    struct token tok;
    struct parserContext* varSource = pc;
    if ((import = tryParseImportAlias(pc, &varSource, varTok))) parseToken(pc, &tok, TOKEN_DOT, "expected .");
    if (!tryParseToken(pc, &tok, TOKEN_IDENTIFIER)) {
        if (import) TokenUnget(pc->tc, 2);
        return NULL;
    }
    *name = tok.str;
    if (import) *varTok = TokenMerge(*varTok, tok);
    else *varTok = tok;

    if (varListContains(&(varSource->publVars), tok.str)) {
        return varListGet(&(varSource->publVars), tok.str);
    }
    else if (!import && varListContains(&(varSource->privVars), tok.str)) {
        return varListGet(&(varSource->privVars), tok.str);
    }
    if (import) TokenUnget(pc->tc, 3);
    else TokenUnget(pc->tc, 1);
    return NULL;
}


bool tryParseVar(struct parserContext* pc, struct variable* v) {
    struct str name;
    struct token varTok;
    struct variable* vPtr = tryParseVarIdentifier(pc, &name, &varTok);
    if (!vPtr) return false;
    *v = *vPtr;
    v->name = name;
    v->tok = varTok;
    return true;
}


struct variable parseVar(struct parserContext* pc) {
    struct variable v;
    if (tryParseVar(pc, &v)) return v;
    SyntaxErrorInvalidToken(TokenNext(pc->tc), "expected type");
    return v;
}


struct variable varNew(struct token nameTok, struct type t) {
    struct variable v;
    v.name = nameTok.str;
    v.tok = nameTok;
    v.type = t;
    v.value = NULL;
    return v;
}


void parseStructTypeDefMember(struct parserContext* pc, struct str structName, struct varList* members) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_IDENTIFIER, "expected variable identifier");
    if (varListContains(members, tok.str)) SyntaxErrorInvalidToken(tok, "duplicate member name");
    struct type t = parseType(pc);
    if (t.bType == BASETYPE_STRUCT && !(t.ref)) {
        if (StrEqual(structName, t.name)) {
            SyntaxErrorInvalidToken(t.tok, "structures may not instantiate themselves");
        }
        else if (typeListContains(&(t.def->embeddedStructs), structName)) {
            SyntaxErrorInvalidToken(t.tok, "circular struct instantiation");
        }
    }
    varListAppend(members, varNew(tok, t));
}


struct varList parseStructTypeDefMembers(struct parserContext* pc, struct str structName) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, NULL);
    struct varList members = varListNew();
    if (tryParseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE)) return members;
    parseStructTypeDefMember(pc, structName, &members);
    while (tryParseToken(pc, &tok, TOKEN_COMMA)) {
        parseStructTypeDefMember(pc, structName, &members);
    }
    parseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, "expected \",\" or \"}\"");
    return members;
}


void parseStructTypeDef(struct parserContext* pc, struct token nameTok) {
    struct varList members = parseStructTypeDefMembers(pc, nameTok.str);
    updateType(pc, structType(nameTok, members));
}


void parseVocabTypeDefMember(struct parserContext* pc, struct strList* list) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_IDENTIFIER, "expected word");
    if (StrListExists(list, tok.str)) SyntaxErrorInvalidToken(tok, "duplicate vocabulary word");
    StrListAppend(list, tok.str);
}


struct strList parseVocabTypeDefMembers(struct parserContext* pc) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, NULL);
    struct strList members = StrListNew();
    parseVocabTypeDefMember(pc, &members);
    while (tryParseToken(pc, &tok, TOKEN_COMMA)) {
        parseVocabTypeDefMember(pc, &members);
    }
    parseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, "expected \",\" or \"}\"");
    return members;
}


void parseVocabTypeDef(struct parserContext* pc, struct token nameTok) {
    struct strList words = parseVocabTypeDefMembers(pc);
    updateType(pc, vocabType(nameTok, words));
}


struct type parseTypeArrAndStructMustBeRef(struct parserContext* pc) {
    struct type t = parseType(pc);
    if (t.bType == BASETYPE_ARRAY && !t.ref) {
        SyntaxErrorInvalidToken(t.tok, "array must be a reference");
    }
    else if (t.bType == BASETYPE_STRUCT && !t.ref) {
        SyntaxErrorInvalidToken(t.tok, "struct must be a reference");
    }
    return t;
}


struct type parseFuncArgType(struct parserContext* pc) {
    bool mut = false;
    struct token tok;
    if (tryParseToken(pc, &tok, TOKEN_MUT)) mut = true;
    struct type t = parseTypeArrAndStructMustBeRef(pc);
    if (!mut) return t;
    if (t.bType != BASETYPE_ARRAY && t.bType != BASETYPE_STRUCT) {
        SyntaxErrorInvalidToken(t.tok, "only arrays or structures may be declared mutable in function args");
    }
    t.mut = true;
    return t;
}


struct varList parseFuncTypeDefArgs(struct parserContext* pc) {
    struct varList args = varListNew();
    struct token tok;

    parseToken(pc, &tok, TOKEN_PAREN_OPEN, NULL);
    if (tryParseToken(pc, &tok, TOKEN_PAREN_CLOSE)) return args;
    varListAppend(&args, varNew(TOKEN_UNDEFINED, parseFuncArgType(pc)));

    while (tryParseToken(pc, &tok, TOKEN_COMMA)) {
        varListAppend(&args, varNew(TOKEN_UNDEFINED, parseFuncArgType(pc)));
    }

    parseToken(pc, &tok, TOKEN_PAREN_CLOSE, "expected \",\" or \")\"");
    return args;
}


struct typeList parseFuncRets(struct parserContext* pc) {
    struct typeList rets = typeListNew();
    struct token tok;

    parseToken(pc, &tok, TOKEN_PAREN_OPEN, NULL);
    if (tryParseToken(pc, &tok, TOKEN_PAREN_CLOSE)) return rets;
    typeListAppend(&rets, parseType(pc));

    while (tryParseToken(pc, &tok, TOKEN_COMMA)) {
        typeListAppend(&rets, parseType(pc));
    }

    parseToken(pc, &tok, TOKEN_PAREN_CLOSE, "expected \",\" or \")\"");
    return rets;
}


void parseFuncTypeDef(struct parserContext* pc, struct token nameTok) {
    struct varList args = parseFuncTypeDefArgs(pc);
    struct typeList rets = parseFuncRets(pc);
    updateType(pc, funcType(nameTok, args, rets));
}


bool typeDependantsContainsInternal(struct typePtrList* searched, struct type* head, struct type* searchFor) {
    for (int i = 0; i < head->dependants.len; i++) {
        if (head->dependants.ptr[i] == searchFor) return true;
        if (typePtrListContainsPtr(searched, head->dependants.ptr[i])) continue;
        typePtrListAppend(searched, head->dependants.ptr[i]);
        if (typeDependantsContainsInternal(searched, head->dependants.ptr[i], searchFor)) return true;
    }
    return false;
}


bool typeDependantsContains(struct type* searchIn, struct type* searchFor) {
    struct typePtrList searched = typePtrListNew();
    return typeDependantsContainsInternal(&searched, searchIn, searchFor);
}


bool baseTypeComplete(struct type* t) {
    if (t->bType == BASETYPE_ARRAY) {
        if (t->arrBType != BASETYPE_PLACEHOLDER) return true;
    }
    else if (t->bType != BASETYPE_PLACEHOLDER) return true;
    return false;
}


void parseAliasTypeDef(struct parserContext* pc, struct token nameTok) {
    struct str name;
    struct token refTok;
    struct type* referenced = tryParseTypeIdentifier(pc, &name, &refTok);
    if (!referenced) SyntaxErrorInvalidToken(TokenNext(pc->tc), "unknown type");

    struct type* alias = pcGetTypePtr(pc, nameTok.str);
    if (baseTypeComplete(referenced)) {
        updateType(pc, aliasType(nameTok, *referenced, alias->dependants));
    }
    else if (typeDependantsContains(alias, referenced)) {
        SyntaxErrorInvalidToken(refTok, "circular type definition");
    }
    else typePtrListAppend(&(referenced->dependants), alias);
}


void typeDependantsComplete(struct type* t) {
    for (int i = t->dependants.len -1; i >= 0; i--) {
        struct type* dep = t->dependants.ptr[i];
        *dep = aliasType(dep->tok, *t, dep->dependants);
        t->dependants.len--;
        typeDependantsComplete(dep);
    }
}


void parseTypeDefComplete(struct parserContext* pc) {
    struct token nameTok;
    parseToken(pc, &nameTok, TOKEN_IDENTIFIER, "expected type name");

    struct token tok;
    tok = TokenNext(pc->tc);
    switch (tok.type) {
        case TOKEN_IDENTIFIER: TokenUnget(pc->tc, 1); parseTypeArrAndStructMustBeRef(pc); break;
        case TOKEN_STRUCT: parseStructTypeDef(pc, nameTok); break;
        case TOKEN_VOCAB: parseVocabTypeDef(pc, nameTok); break;
        case TOKEN_FUNC: parseFuncTypeDef(pc, nameTok); break;
        default: SyntaxErrorInvalidToken(tok, "not a type"); exit(EXIT_FAILURE);
    }
}


struct parserContext parserContextNew(struct str fileName, struct pcList* parsedFiles) {
    struct parserContext pc;
    pc.fileName = fileName;
    pc.tc = TokenContextNew(fileName);
    pc.parsedFiles = parsedFiles;
    pc.publTypes = typeListNew();
    pc.privTypes = typeListNew();
    pc.publVars = varListNew();
    pc.privVars = varListNew();
    return pc;
}


void parseFileFirstPass(struct parserContext* pc);


bool pcContainsFileName(struct parserContext* pc, struct str fileName) {
    for (int i = 0; i < pc->publVars.len; i++) {
        struct variable var = pc->publVars.ptr[i];
        if(var.type.bType == BASETYPE_IMPORT  && StrEqual(var.value->arrVal, fileName)) {
            return true;
        }
    }
    return false;
}


void parseImportFirstPass(struct parserContext* pc) {
    struct token fileNameTok;
    struct token aliasTok;
    parseToken(pc, &fileNameTok, TOKEN_STRING, "expected import file name string");
    struct str fileName = StrSlice(fileNameTok.str, 1, StrGetLen(fileNameTok.str) - 1);
    if (StrEqual(fileName, pc->fileName)) {
        SyntaxErrorInvalidToken(fileNameTok, "files may not import themselves");
    }
    if (pcContainsFileName(pc, fileName)) {
        SyntaxErrorInvalidToken(fileNameTok, "file already imported in this name space");
    }
    parseToken(pc, &aliasTok, TOKEN_IDENTIFIER, "expected import alias");
    if (pcContainsVar(pc, aliasTok.str)) {
        SyntaxErrorInvalidToken(aliasTok, "file alias already in use");
    }
    if (!pcListContains(pc->parsedFiles, fileName)) {
        pcListAppend(pc->parsedFiles, parserContextNew(fileName, pc->parsedFiles));
        struct parserContext* importPc = pcListGet(pc->parsedFiles, fileName);
        parseFileFirstPass(importPc);
    }
    struct variable aliasVar = varNew(aliasTok, importType());
    aliasVar.value = operandStringLiteral(fileNameTok);
    pcAddVar(pc, aliasVar);
}


void parseGlobalLevel(struct parserContext* pc) {
    struct token tok = TokenNext(pc->tc);
    switch (tok.type) {
        case TOKEN_TYPE: break;
        case TOKEN_FUNC: break;
        default: //SyntaxErrorInvalidToken(tok, NULL); 
    }
}


void parseTypePlaceholder(struct parserContext* pc) {
    struct token nameTok;
    parseToken(pc, &nameTok, TOKEN_IDENTIFIER, "expected type name");
    pcAddType(pc, placeholderType(nameTok));
}


void parseTypePlaceholdersAndImportsFirstPass(struct parserContext* pc) {
    struct token tok;
    while ((tok = TokenNext(pc->tc)).type != TOKEN_EOF) {
        if (tok.type == TOKEN_TYPE) parseTypePlaceholder(pc);
        else if (tok.type == TOKEN_IMPORT) parseImportFirstPass(pc);
    }
}


//basetypes are not complete until function has been called with all imported files too
void parseBaseTypes(struct parserContext* pc) {
    struct token nameTok;
    struct token tok;
    while ((tok = TokenNext(pc->tc)).type != TOKEN_EOF) {
        if (tok.type == TOKEN_TYPE) {
            parseToken(pc, &nameTok, TOKEN_IDENTIFIER, "expected type name");
            struct type* t = pcGetTypePtr(pc, nameTok.str);
            tok = TokenNext(pc->tc);
            switch(tok.type) {
                case TOKEN_IDENTIFIER: TokenUnget(pc->tc, 1); parseAliasTypeDef(pc, nameTok); break;
                case TOKEN_STRUCT: t->bType = BASETYPE_STRUCT; break;
                case TOKEN_VOCAB: t->bType = BASETYPE_VOCAB; break;
                case TOKEN_FUNC: t->bType = BASETYPE_FUNC; break;
                default: SyntaxErrorInvalidToken(tok, "expected type");
            }
            if (baseTypeComplete(t)) typeDependantsComplete(t); 
        }
    }
}


void parseFuncArgVarDeclaration(struct parserContext* pc, struct varList* args) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_IDENTIFIER, "expected variable identifier");
    if (isPublic(tok.str)) SyntaxErrorInvalidToken(tok, "local variables may not be capitalized");
    if (varListContains(args, tok.str)) SyntaxErrorInvalidToken(tok, "duplicate argument name");
    varListAppend(args, varNew(tok, parseFuncArgType(pc)));
}


struct varList parseFuncArgs(struct parserContext* pc) {
    struct varList args = varListNew();
    struct token tok;
    parseToken(pc, &tok, TOKEN_PAREN_OPEN, NULL);
    if (tryParseToken(pc, &tok, TOKEN_PAREN_CLOSE)) return args;
    parseFuncArgVarDeclaration(pc, &args);
    while (tryParseToken(pc, &tok, TOKEN_COMMA)) parseFuncArgVarDeclaration(pc, &args);
    parseToken(pc, &tok, TOKEN_PAREN_CLOSE, "expected \",\" or \")\"");
    return args;
}


void parseFuncHeader(struct parserContext* pc) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_IDENTIFIER, "expected function name");
    struct varList args = parseFuncArgs(pc);
    struct typeList rets = parseFuncRets(pc);
    struct type funcT = funcType(tok, args, rets);
    pcAddVar(pc, varNew(tok, funcT));
}


void parseCompleteTypesAndFuncHeaders(struct parserContext* pc) {
    struct token tok;
    while ((tok = TokenNext(pc->tc)).type != TOKEN_EOF) {
        if (tok.type == TOKEN_TYPE) parseTypeDefComplete(pc);
        else if (tok.type == TOKEN_FUNC) parseFuncHeader(pc);
    }
}


void parseFileFirstPass(struct parserContext* pc) {
    parseTypePlaceholdersAndImportsFirstPass(pc);
    TokenRestart(pc->tc);
}


void parseFileSecondPass(struct parserContext* pc) {
    parseBaseTypes(pc);
    TokenRestart(pc->tc);
}


void parseFileThirdPass(struct parserContext* pc) {
    parseCompleteTypesAndFuncHeaders(pc);
    TokenRestart(pc->tc);
}


void parseFileFourthPass(struct parserContext* pc) {
    struct token tok;
    while (!tryParseToken(pc, &tok, TOKEN_EOF)) {
        parseGlobalLevel(pc);
    }
}


struct type vanillaType(char* name, enum baseType bType) {
    struct typeDef* tDef = typeDefEmpty();
    tDef->complete = true;

    struct type t = {0};
    t.name = StrFromCharArray(name);
    t.tok = TOKEN_UNDEFINED;
    t.bType = bType;
    t.def = tDef;
    t.ref = true;
    return t;
}


void initVanillaTypes() {
    vanillaTypeBool = vanillaType("bool", BASETYPE_BOOL);
    vanillaTypeBit8 = vanillaType("bit8", BASETYPE_BIT8);
    vanillaTypeBit16 = vanillaType("bit16", BASETYPE_BIT16);
    vanillaTypeBit32 = vanillaType("bit32", BASETYPE_BIT32);
    vanillaTypeBit64 = vanillaType("bit64", BASETYPE_BIT64);
    vanillaTypeInt8 = vanillaType("int8", BASETYPE_INT8);
    vanillaTypeInt16 = vanillaType("int16", BASETYPE_INT16);
    vanillaTypeInt32 = vanillaType("int32", BASETYPE_INT32);
    vanillaTypeInt64 = vanillaType("int64", BASETYPE_INT64);
    vanillaTypeFloat32 = vanillaType("float32", BASETYPE_FLOAT32);
    vanillaTypeFloat64 = vanillaType("float64", BASETYPE_FLOAT64);
}


void ParseMainFile(char* fileName) {
    initVanillaTypes();
    struct pcList parsedFiles = pcListNew();
    pcListAppend(&parsedFiles, parserContextNew(StrFromCharArray(fileName), &parsedFiles));
    struct parserContext* pc = pcListGet(&parsedFiles, StrFromCharArray(fileName));
    parseFileFirstPass(pc);
    for (int i = 0; i < parsedFiles.len; i++) {
        parseFileSecondPass(parsedFiles.ptr + i);
    }
    for (int i = 0; i < parsedFiles.len; i++) {
        parseFileThirdPass(parsedFiles.ptr + i);
    }
    for (int i = 0; i < parsedFiles.len; i++) {
        parseFileFourthPass(parsedFiles.ptr + i);
    }
}
