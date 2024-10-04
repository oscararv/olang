#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "str.h"
#include "token.h"
#include "error.h"


struct typeDef intConstTypeDef = {.complete = true};
struct type intConstType = {.tok = TOKEN_UNDEFINED, .bType = BASETYPE_INT, .def = &intConstTypeDef};

struct typeDef floatConstTypeDef = {.complete = true};
struct type floatConstType = {.tok = TOKEN_UNDEFINED, .bType = BASETYPE_FLOAT, .def = &floatConstTypeDef};

struct typeDef charConstTypeDef = {.complete = true};
struct type charConstType = {.tok = TOKEN_UNDEFINED, .bType = BASETYPE_CHAR, .def = &charConstTypeDef};

struct typeDef stringConstTypeDef = {.complete = true};
struct type stringConstType = {.tok = TOKEN_UNDEFINED, .bType = BASETYPE_FLOAT, .def = &stringConstTypeDef};


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
    if (opl->len >= opl->cap) {
        opl->cap += 100;
        opl->ptr = realloc(opl->ptr, sizeof(*(opl->ptr)) * opl->cap);
        CheckPtr(opl->ptr);
    }
    opl->ptr[opl->len] = o;
    (opl->len)++;
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


bool typeListContains(struct typeList* tl, struct str name) {
    for (int i = 0; i < tl->len; i++) {
        if (StrEqual(tl->ptr[i].name, name)) return true;
    }
    return false;
}


bool typePtrListContains(struct typePtrList* tpl, struct str name) {
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


struct type typeListGet(struct typeList* tl, struct str name) {
    for (int i = 0; i < tl->len; i++) {
        if (StrEqual(tl->ptr[i].name, name)) return tl->ptr[i];
    }
    Error("type does not exist in type list");
    exit(EXIT_FAILURE); //unreachable
}


struct type* typeListGetPtr(struct typeList* tl, struct str name) {
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


struct variable varListGet(struct varList* vl, struct str name) {
    for (int i = 0; i < vl->len; i++) {
        if (StrEqual(vl->ptr[i].name, name)) return vl->ptr[i];
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


void appendVanillaTypes(struct typeList* spl) {
    typeListAppend(spl, vanillaType("byte", BASETYPE_BYTE));
    typeListAppend(spl, vanillaType("bool", BASETYPE_BOOL));
    typeListAppend(spl, vanillaType("int8", BASETYPE_INT8));
    typeListAppend(spl, vanillaType("int16", BASETYPE_INT16));
    typeListAppend(spl, vanillaType("int32", BASETYPE_INT32));
    typeListAppend(spl, vanillaType("int64", BASETYPE_INT64));
    typeListAppend(spl, vanillaType("uint8", BASETYPE_UINT8));
    typeListAppend(spl, vanillaType("uint16", BASETYPE_UINT16));
    typeListAppend(spl, vanillaType("uint32", BASETYPE_UINT32));
    typeListAppend(spl, vanillaType("uint64", BASETYPE_UINT64));
    typeListAppend(spl, vanillaType("float32", BASETYPE_FLOAT32));
    typeListAppend(spl, vanillaType("float64", BASETYPE_FLOAT64));
}


bool pcContainsType(struct parserContext* pc, struct str name) {
    if (isPublic(name) && typeListContains(&(pc->publTypes), name)) return true;
    return typeListContains(&(pc->privTypes), name);
}


bool pcContainsVar(struct parserContext* pc, struct str name) {
    if (isPublic(name) && varListContains(&(pc->publVarsAndConsts), name)) return true;
    return varListContains(&(pc->privVarsAndConsts), name);
}


bool pcContainsSymbol(struct parserContext* pc, struct str name) {
    if (pcContainsType(pc, name)) return true;
    return pcContainsVar(pc, name);
}


struct variable pcGetVar(struct parserContext* pc, struct str name) {
    if (isPublic(name)) return varListGet(&(pc->publVarsAndConsts), name);
    return varListGet(&(pc->privVarsAndConsts), name);
}


struct type* pcGetTypePtr(struct parserContext* pc, struct str name) {
    if (isPublic(name)) return typeListGetPtr(&(pc->publTypes), name);
    return typeListGetPtr(&(pc->privTypes), name);
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
    if (isPublic(v.name)) varListAppend(&(pc->publVarsAndConsts), v);
    else varListAppend(&(pc->privVarsAndConsts), v);
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
    t.name = TOKEN_UNDEFINED.str;
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


bool tryParseToken(struct parserContext* pc, struct token* tok,
        enum tokenType type, bool discardNewlines) {

    struct token tmpTok;
    if (discardNewlines) {
        tmpTok = TokenNextDiscardNewlines(pc->tc);
    }
    else tmpTok = TokenNext(pc->tc);
    if (tmpTok.type != type) {
        TokenUnget(pc->tc);
        return false;
    }
    *tok = tmpTok;
    return true;
}


//errorMsg is autogenerated on NULL
void parseToken(struct parserContext* pc, struct token* tok,
        enum tokenType type, bool discardNewlines, char* errorMsg) {

    if (discardNewlines) {
        *tok = TokenNextDiscardNewlines(pc->tc);
    }
    else *tok = TokenNext(pc->tc);
    if (tok->type != type) {
        if (errorMsg == NULL) {
            char msg[100];
            strcpy(msg, "expected ");
            strcat(msg, TokenTypeToString(type));
            errorMsg = msg;
        }
        SyntaxErrorInvalidToken(*tok, errorMsg);
    }
}


struct operand* operandEmpty() {
    struct operand* op = malloc(sizeof(*op));
    CheckPtr(op);
    *op = (struct operand){0};
    op->tok = TOKEN_UNDEFINED;
    return op;
}


struct operand* operandIntConstNoToken(long long intVal) {
    struct operand* op = operandEmpty();
    op->tok = TOKEN_UNDEFINED;
    op->type = intConstType;
    op->valKnown = true;
    op->intVal = intVal;
    op->operation = OPERATION_NOOP;
    return op;

}


struct operand* operandIntConst(struct token valueTok) {
    struct operand* op = operandIntConstNoToken(StrToLongLongVal(valueTok.str));
    op->tok = valueTok;
    return op;
}


struct operand* operandFloatConst(struct token valueTok) {
    struct operand* op = operandEmpty();
    op->tok = valueTok;
    op->type = floatConstType;
    op->valKnown = true;
    op->floatVal = StrToDoubleVal(valueTok.str);
    op->operation = OPERATION_NOOP;
    return op;
}


struct operand* operandCharConst(struct token valueTok) {
    struct operand* op = operandEmpty();
    op->tok = valueTok;
    op->type = charConstType;
    op->valKnown = true;
    op->intVal = StrToCharVal(valueTok.str);
    op->operation = OPERATION_NOOP;
    return op;
}


struct operand* operandStringConst(struct token valueTok) {
    struct operand* op = operandEmpty();
    op->tok = valueTok;
    op->type = stringConstType;
    op->valKnown = true;
    op->strVal = StrToStringVal(valueTok.str);
    op->operation = OPERATION_NOOP;
    return op;
}


bool parseIntConstantExpr(struct parserContext* pc, struct operand** op, bool discardNewlines) {
    //TODO parse expr not just single operand
    struct token tok;
    parseToken(pc, &tok, TOKEN_INT, discardNewlines, NULL);
    *op = operandIntConst(tok);
    return false;
}


struct parserContext* getImportPc(struct parserContext* head, struct str alias) {
    struct variable aliasVar = pcGetVar(head, alias);
    return pcListGet(head->parsedFiles, aliasVar.value->strVal);
}


bool tryParseImportAlias(struct parserContext* pc, struct parserContext** import, struct token* aliasTok) {
    struct token tok;
    if (!tryParseToken(pc, &tok, TOKEN_IDENTIFIER, false)) return false;
    if (!pcContainsVar(pc, tok.str)) {
        TokenUnget(pc->tc);
        return false;
    }
    *aliasTok = tok;
    *import = getImportPc(pc, tok.str);
    return true;
}


struct type parseTypeIdentifier(struct parserContext* pc, struct str* name, struct token* typeToken) {
    bool import;
    struct token tok;
    struct parserContext* typeSource = pc;
    if ((import = tryParseImportAlias(pc, &typeSource, typeToken))) {
        parseToken(pc, &tok, TOKEN_DOT, false, "expected .");
    }
    parseToken(pc, &tok, TOKEN_IDENTIFIER, false, "unknown type");
    *name = tok.str;
    if (import) *typeToken = TokenExtend(*typeToken, tok);
    else *typeToken = tok;

    if (typeListContains(&(typeSource->publTypes), tok.str)) {
        return typeListGet(&(typeSource->publTypes), tok.str);
    }
    else if (!import && typeListContains(&(typeSource->privTypes), tok.str)) {
        return typeListGet(&(typeSource->privTypes), tok.str);
    }
    else SyntaxErrorInvalidToken(*typeToken, "unknown type");
    exit(EXIT_FAILURE); //unreachable
}


void parseTypeIsStructInstantiation(struct parserContext* pc, struct type* t) {
    struct token tok;
    if (t->bType == BASETYPE_STRUCT && tryParseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, false)) {
        parseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, false, NULL);
        t->tok = TokenExtend(t->tok, tok);
        t->ref = false;
    }
}


void parseTypeIsArray(struct parserContext* pc, struct type* t) {
    struct token tok;
    bool isArray = false;
    while (tryParseToken(pc, &tok, TOKEN_SQUAREBRACKET_OPEN, false)) {
        isArray = true;
        if (tryParseToken(pc, &tok, TOKEN_SQUAREBRACKET_CLOSE, false)) {
            opPtrListAppend(&(t->arrLenghts), operandIntConstNoToken(ARRAY_REF));
            t->tok = TokenExtend(t->tok, tok);
        }
        else {
            struct operand* arrLen;
            parseIntConstantExpr(pc, &arrLen, false);
            opPtrListAppend(&(t->arrLenghts), arrLen);
            parseToken(pc, &tok, TOKEN_SQUAREBRACKET_CLOSE, false, NULL);
            t->tok = TokenExtend(t->tok, tok);
        }
    }
    if (isArray) {
        t->arrBType = t->bType;
        t->bType = BASETYPE_ARRAY;
        if (t->arrLenghts.ptr[0]->intVal != ARRAY_REF) t->ref = false;
    }
}


struct type parseType(struct parserContext* pc) {
    struct token tok;
    struct str name;
    struct type t = parseTypeIdentifier(pc, &name, &tok);
    t.name = name;
    t.tok = tok;
    parseTypeIsStructInstantiation(pc, &t);
    parseTypeIsArray(pc, &t);
    return t;
}


struct variable parseVar(struct parserContext* pc, struct token* varTok) {
    bool alias;
    struct token tok;
    struct parserContext* import;
    struct parserContext* varSource = pc;
    if ((alias = tryParseImportAlias(pc, &import, varTok))) {
        parseToken(pc, &tok, TOKEN_DOT, false, "expected .");
        varSource = import;
    }

    parseToken(pc, &tok, TOKEN_IDENTIFIER, false, "unknown variable");
    if (alias) *varTok = TokenExtend(*varTok, tok);
    else *varTok = tok;

    if (varListContains(&(varSource->publVarsAndConsts), tok.str)) {
        return varListGet(&(varSource->publVarsAndConsts), tok.str);
    }
    else if (!alias && varListContains(&(varSource->privVarsAndConsts), tok.str)) {
        return varListGet(&(varSource->privVarsAndConsts), tok.str);
    }
    else SyntaxErrorInvalidToken(*varTok, "unknown variable");
    exit(EXIT_FAILURE); //unreachable
}


struct operand* parseOperand(struct parserContext* pc, struct varList* localVars) {
    struct token tok = TokenNext(pc->tc);
    if (varListContains(localVars, tok.str)) return varListGet(localVars, tok.str).value;
    else if (tok.type == TOKEN_IDENTIFIER) {
        TokenUnget(pc->tc);
        return parseVar(pc, &tok).value;
    }
    else {
        if (tok.type == TOKEN_INT) return operandIntConst(tok);
        if (tok.type == TOKEN_FLOAT) return operandFloatConst(tok);
        if (tok.type == TOKEN_CHAR) return operandCharConst(tok);
        if (tok.type == TOKEN_STRING) return operandStringConst(tok);
        SyntaxErrorInvalidToken(tok, "expected variable identifier or constant literal");
    }
    exit(EXIT_FAILURE); //unreachable
}


enum operationType parseOperator(struct parserContext* pc) {
    struct token tok = TokenNext(pc->tc);
    switch(tok.type) {
        case TOKEN_IDENTIFIER: return OPERATION_TYPECAST; //TODO func call
        case TOKEN_MUL: return OPERATION_MUL;
        case TOKEN_DIV: return OPERATION_DIV;
        case TOKEN_ADD: return OPERATION_ADD;
        case TOKEN_SUB: return OPERATION_SUB;
        case TOKEN_MODULO: return OPERATION_MODULO;
        case TOKEN_INCREMENT: return OPERATION_INCREMENT;
        case TOKEN_DECREMENT: return OPERATION_DECREMENT;
        case TOKEN_LOGICAL_EQUALS: return OPERATION_LOGICAL_EQUALS;
        case TOKEN_LOGICAL_NOT: return OPERATION_LOGICAL_NOT;
        case TOKEN_LOGICAL_AND: return OPERATION_LOGICAL_AND;
        case TOKEN_LOGICAL_OR: return OPERATION_LOGICAL_OR;
        case TOKEN_LOGICAL_LESS_THAN: return OPERATION_LOGICAL_LESS_THAN;
        case TOKEN_LOGICAL_LESS_THAN_OR_EQUAL: return OPERATION_LOGICAL_LESS_THAN_OR_EQUAL;
        case TOKEN_LOGICAL_GREATER_THAN: return OPERATION_LOGICAL_GREATER_THAN;
        case TOKEN_LOGICAL_GREATER_THAN_OR_EQUAL: return OPERATION_LOGICAL_GREATER_THAN_OR_EQUAL;
        case TOKEN_BITWISE_AND: return OPERATION_BITWISE_AND;
        case TOKEN_BITWISE_OR: return OPERATION_BITWISE_OR;
        case TOKEN_BITWISE_XOR: return OPERATION_BITWISE_XOR;
        case TOKEN_BITWISE_COMPLEMENT: return OPERATION_BITWISE_COMPLEMENT;
        case TOKEN_BITSHIFT_LEFT: return OPERATION_BITSHIFT_LEFT;
        case TOKEN_BITSHIFT_RIGHT: return OPERATION_BITSHIFT_RIGHT;
        default: SyntaxErrorInvalidToken(tok, "expected operator"); exit(EXIT_FAILURE);
    }
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
    parseToken(pc, &tok, TOKEN_IDENTIFIER, true, "expected variable identifier");
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
    parseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, false, NULL);
    struct varList members = varListNew();
    if (tryParseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, true)) return members;
    parseStructTypeDefMember(pc, structName, &members);
    while (tryParseToken(pc, &tok, TOKEN_COMMA, false)) {
        parseStructTypeDefMember(pc, structName, &members);
    }
    parseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, true, "expected \",\" or \"}\"");
    return members;
}


void parseStructTypeDef(struct parserContext* pc, struct token nameTok) {
    struct varList members = parseStructTypeDefMembers(pc, nameTok.str);
    updateType(pc, structType(nameTok, members));
}


void parseVocabTypeDefMember(struct parserContext* pc, struct strList* list) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_IDENTIFIER, true, "expected word");
    if (StrListExists(list, tok.str)) SyntaxErrorInvalidToken(tok, "duplicate vocabulary word");
    StrListAppend(list, tok.str);
}


struct strList parseVocabTypeDefMembers(struct parserContext* pc) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, false, NULL);
    struct strList members = StrListNew();
    parseVocabTypeDefMember(pc, &members);
    while (tryParseToken(pc, &tok, TOKEN_COMMA, false)) {
        TokenDiscardNewlines(pc->tc);
        parseVocabTypeDefMember(pc, &members);
    }
    parseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, true, "expected \",\" or \"}\"");
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
    if (tryParseToken(pc, &tok, TOKEN_MUT, true)) mut = true;
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

    parseToken(pc, &tok, TOKEN_PAREN_OPEN, true, NULL);
    if (tryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true)) return args;
    varListAppend(&args, varNew(TOKEN_UNDEFINED, parseFuncArgType(pc)));

    while (tryParseToken(pc, &tok, TOKEN_COMMA, false)) {
        varListAppend(&args, varNew(TOKEN_UNDEFINED, parseFuncArgType(pc)));
    }

    parseToken(pc, &tok, TOKEN_PAREN_CLOSE, true, "expected \",\" or \")\"");
    return args;
}


struct typeList parseFuncRets(struct parserContext* pc) {
    struct typeList rets = typeListNew();
    struct token tok;

    parseToken(pc, &tok, TOKEN_PAREN_OPEN, true, NULL);
    if (tryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true)) return rets;
    typeListAppend(&rets, parseType(pc));

    while (tryParseToken(pc, &tok, TOKEN_COMMA, false)) {
        TokenDiscardNewlines(pc->tc);
        typeListAppend(&rets, parseType(pc));
    }

    parseToken(pc, &tok, TOKEN_PAREN_CLOSE, true, "expected \",\" or \")\"");
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


struct type* parseAliasReferenceTypePtr(struct parserContext* pc, struct token* refToken) {
    bool import;
    struct token tok;
    struct parserContext* typeSource = pc;
    if ((import = tryParseImportAlias(pc, &typeSource, refToken))) {
        parseToken(pc, &tok, TOKEN_DOT, false, NULL);
    }
    parseToken(pc, &tok, TOKEN_IDENTIFIER, false, "unknown type");
    if (import) *refToken = TokenExtend(*refToken, tok);
    else *refToken = tok;

    if (typeListContains(&(typeSource->publTypes), tok.str)) {
        return typeListGetPtr(&(typeSource->publTypes), tok.str);
    }
    else if (!import && typeListContains(&(typeSource->privTypes), tok.str)) {
        return typeListGetPtr(&(typeSource->privTypes), tok.str);
    }
    else SyntaxErrorInvalidToken(*refToken, "unknown type");
    exit(EXIT_FAILURE); //unreachable
}


void parseAliasTypeDef(struct parserContext* pc, struct token nameTok) {
    struct token refTok;
    struct type* referenced = parseAliasReferenceTypePtr(pc, &refTok);
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
    parseToken(pc, &nameTok, TOKEN_IDENTIFIER, false, "expected type name");

    struct token tok;
    tok = TokenNext(pc->tc);
    switch (tok.type) {
        case TOKEN_IDENTIFIER: TokenUnget(pc->tc); parseTypeArrAndStructMustBeRef(pc); break;
        case TOKEN_STRUCT: parseStructTypeDef(pc, nameTok); break;
        case TOKEN_VOCAB: parseVocabTypeDef(pc, nameTok); break;
        case TOKEN_FUNC: parseFuncTypeDef(pc, nameTok); break;
        default: SyntaxErrorInvalidToken(tok, "not a type"); exit(EXIT_FAILURE);
    }
    parseToken(pc, &tok, TOKEN_NEWLINE, false, NULL);
}


struct parserContext parserContextNew(struct str fileName, struct pcList* parsedFiles) {
    struct parserContext pc;
    pc.fileName = fileName;
    pc.tc = TokenContextNew(fileName);
    pc.parsedFiles = parsedFiles;
    pc.publTypes = typeListNew();
    pc.privTypes = typeListNew();
    pc.publVarsAndConsts = varListNew();
    pc.privVarsAndConsts = varListNew();
    appendVanillaTypes(&(pc.privTypes));
    return pc;
}


void parseFileFirstPass(struct parserContext* pc);


bool pcContainsFileName(struct parserContext* pc, struct str fileName) {
    for (int i = 0; i < pc->publVarsAndConsts.len; i++) {
        struct variable var = pc->publVarsAndConsts.ptr[i];
        if(var.type.bType == BASETYPE_IMPORT  && StrEqual(var.value->strVal, fileName)) {
            return true;
        }
    }
    return false;
}


void parseImportFirstPass(struct parserContext* pc) {
    struct token fileNameTok;
    struct token aliasTok;
    parseToken(pc, &fileNameTok, TOKEN_STRING, false, "expected import file name string");
    struct str fileName = StrSlice(fileNameTok.str, 1, StrGetLen(fileNameTok.str) - 1);
    if (StrEqual(fileName, pc->fileName)) {
        SyntaxErrorInvalidToken(fileNameTok, "files may not import themselves");
    }
    if (pcContainsFileName(pc, fileName)) {
        SyntaxErrorInvalidToken(fileNameTok, "file already imported in this name space");
    }
    parseToken(pc, &aliasTok, TOKEN_IDENTIFIER, false, "expected import alias");
    if (pcContainsVar(pc, aliasTok.str)) {
        SyntaxErrorInvalidToken(aliasTok, "file alias already in use");
    }
    if (!pcListContains(pc->parsedFiles, fileName)) {
        pcListAppend(pc->parsedFiles, parserContextNew(fileName, pc->parsedFiles));
        struct parserContext* importPc = pcListGet(pc->parsedFiles, fileName);
        parseFileFirstPass(importPc);
    }
    struct variable aliasVar = varNew(aliasTok, importType());
    aliasVar.value = operandStringConst(fileNameTok);
    pcAddVar(pc, aliasVar);
}


void parseGlobalLevel(struct parserContext* pc) {
    struct token tok = TokenNextDiscardNewlines(pc->tc);
    switch (tok.type) {
        case TOKEN_TYPE: break;
        case TOKEN_FUNC: break;
        default: //SyntaxErrorInvalidToken(tok, NULL); 
    }
}


void parseTypePlaceholder(struct parserContext* pc) {
    struct token nameTok;
    parseToken(pc, &nameTok, TOKEN_IDENTIFIER, false, "expected type name");
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
            parseToken(pc, &nameTok, TOKEN_IDENTIFIER, false, "expected type name");
            struct type* t = pcGetTypePtr(pc, nameTok.str);
            tok = TokenNext(pc->tc);
            switch(tok.type) {
                case TOKEN_IDENTIFIER: TokenUnget(pc->tc); parseAliasTypeDef(pc, nameTok); break;
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
    parseToken(pc, &tok, TOKEN_IDENTIFIER, false, "expected variable identifier");
    if (isPublic(tok.str)) SyntaxErrorInvalidToken(tok, "local variables may not be capitalized");
    if (varListContains(args, tok.str)) SyntaxErrorInvalidToken(tok, "duplicate argument name");
    varListAppend(args, varNew(tok, parseFuncArgType(pc)));
}


struct varList parseFuncArgs(struct parserContext* pc) {
    struct varList args = varListNew();
    struct token tok;
    parseToken(pc, &tok, TOKEN_PAREN_OPEN, true, NULL);
    if (tryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true)) return args;
    parseFuncArgVarDeclaration(pc, &args);
    while (tryParseToken(pc, &tok, TOKEN_COMMA, true)) parseFuncArgVarDeclaration(pc, &args);
    parseToken(pc, &tok, TOKEN_PAREN_CLOSE, false, "expected \",\" or \")\"");
    return args;
}


void parseFuncHeader(struct parserContext* pc) {
    struct token tok;
    parseToken(pc, &tok, TOKEN_IDENTIFIER, false, "expected function name");
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
    while (!tryParseToken(pc, &tok, TOKEN_EOF, true)) {
        parseGlobalLevel(pc);
    }
}


void ParseMainFile(char* fileName) {
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
