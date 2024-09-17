#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "parser.h"
#include "str.h"
#include "token.h"
#include "error.h"


static bool isPublic(struct str str) {
    if (StrGetLen(str) <= 0) Error("can not check publicity on empty string");
    char c = StrGetChar(str, 0);
    if (c >= 'A' && c <= 'Z') return true;
    return false;
}


static void TypeSetMutTrue(struct type* t) {
    t->mut = true;
}


static struct type BaseType(char* name, enum baseType bType) {
    struct type t;
    t.bType = bType;
    t.name = StrFromCharArray(name);
    t.advanced = NULL;
    t.ref = false;
    t.mut = false;
    return t;
}


struct operandList OperandListSlice(struct operandList ol, int start, int len) {
    if (start < 0 || start > ol.len -1) Error("slice start index out of bounds");
    if (len < 0 || len > ol.len - start) Error("slice len runs out of bounds");

    struct operandList ret;
    ret.isSlice = true;
    ret.len = ol.len - start;
    ret.cap = ol.cap - start;
    ret.ptr = ol.ptr;
    return ret;
}


void TypeListAppend(struct typeList* tl, struct type type) {
    if (tl->len >= tl->cap) {
        tl->cap += 100;
        tl->ptr = realloc(tl->ptr, sizeof(*(tl->ptr)) * tl->cap);
        CheckPtr(tl->ptr);
    }
    tl->ptr[tl->len] = type;
    tl->len++;
}


void TypeListMerge(struct typeList* base, struct typeList appendix) {
    int oldLen = base->len;
    base->len += appendix.len;
    if (base->len >= base->cap) {
        base->cap = appendix.len;
        base->ptr = realloc(base->ptr, sizeof(*(base->ptr)) * base->cap);
        CheckPtr(base->ptr);
    }
    for (int i = 0; i < appendix.len; i++) {
        base->ptr[oldLen + i] = appendix.ptr[i];
    }
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


bool GetType(struct str name, struct type* t, struct parserContext* pc) {
    if (typeListGet(pc->publTypes, name, t)) return true;
    if (typeListGet(pc->privTypes, name, t)) return true;
    return false;
}


static bool typeListExists(struct typeList tl, struct str name) {
    for (int i = 0; i < tl.len; i++) {
        if (StrEqual(tl.ptr[i].name, name)) {
            return true;
        }
    }
    return false;
}


bool ExistsType(struct str name, struct typeList publ, struct typeList priv) {
    if (typeListExists(publ, name)) return true;
    if (typeListExists(priv, name)) return true;
    return false;
}


void typeListUpdate(struct typeList* tl, struct type t) {
    for (int i = 0; i < tl->len; i++) {
        if (StrEqual(t.name, tl->ptr[i].name)) {
            tl->ptr[i].advanced = t.advanced;
            return;
        }
    }
    Error("type to be updated does not exist");
}


void UpdateType(struct parserContext* pc, struct type t) {
    struct typeList* tl = isPublic(t.name) ? &(pc->publTypes) : &(pc->privTypes);
    typeListUpdate(tl, t);
}


void AddType(struct type t, struct parserContext* pc) {
    if (ExistsType(t.name, pc->publTypes, pc->privTypes)) {
        SyntaxErrorInvalidToken(t.tok, "type name already exists");
    }
    if (isPublic(t.name)) TypeListAppend(&(pc->publTypes), t);
    else TypeListAppend(&(pc->privTypes), t);
}


struct type ArrType(struct type heldType, int len, bool ref) {
    struct arrTypeData* arrT = malloc(sizeof(*arrT));
    CheckPtr(arrT);
    arrT->heldType = heldType;
    arrT->len = len;

    struct type t;
    t.bType = BASETYPE_ARRAY;
    t.name = StrNew();
    t.advanced = arrT;
    t.ref = ref;
    t.mut = false;
    return t;
}


struct typeList GetEmbeddedStructMembers(struct operandList members) {
    struct typeList embedded = TypeListNew();
    for (int i = 0; i < members.len; i++) {
        struct type membType = members.ptr[i].type;
        if (membType.bType == BASETYPE_STRUCT && !membType.ref) {
            TypeListAppend(&embedded, membType);
            if (membType.advanced) {
                struct typeList contained =
                GetEmbeddedStructMembers(((struct structTypeData*)membType.advanced)->members);
                TypeListMerge(&embedded, contained);
            }
        }
    }
    return embedded;
}


struct type StructType(struct operandList members) {
    struct structTypeData* structT = malloc(sizeof(*structT));
    CheckPtr(structT);
    structT->members = members;
    structT->embeddedStructs = GetEmbeddedStructMembers(members);

    struct type t;
    t.ref = true;
    t.bType = BASETYPE_STRUCT;
    t.name = StrNew();
    t.advanced = structT;
    t.mut = false;
    return t;
}


struct type VocabType(struct strList words) {
    struct vocabTypeData* vocabT = malloc(sizeof(*vocabT));
    CheckPtr(vocabT);
    vocabT->words = words;

    struct type t;
    t.bType = BASETYPE_VOCAB;
    t.name = StrNew();
    t.advanced = vocabT;
    t.ref = false;
    t.mut = false;
    return t;
}


struct type FuncType(struct typeList args, struct typeList rets) {
    struct funcTypeData* funcT = malloc(sizeof(struct funcTypeData));
    CheckPtr(funcT);
    funcT->args = args;
    funcT->rets = rets;

    struct type t;
    t.bType = BASETYPE_FUNC;
    t.name = StrNew();
    t.advanced = funcT;
    t.ref = false;
    t.mut = false;
    return t;
}


struct type AliasType(struct type old, struct str name) {
    old.name = name;
    return old;
}


struct operand OperandNew(struct str name, struct type type) {
    struct operand op;
    op.name = name;
    op.type = type;
    op.init = false;
    return op;
}


struct operandList OperandListNew() {
    struct operandList ol;
    ol.isSlice = false;
    ol.len = 0;
    ol.cap = 0;
    ol.ptr = NULL;
    return ol;
}


static bool operandListExists(struct operandList ol, struct str name) {
    for (int i = 0; i < ol.len; i++) {
        if (StrEqual(ol.ptr[i].name, name)) {
            return true;
        }
    }
    return false;
}


void operandListAppend(struct operandList* ol, struct operand op) {
    if (ol->isSlice) Error("tried to append to operand slice");
    if (ol->len >= ol->cap) {
        ol->cap += 100;
        ol->ptr = realloc(ol->ptr, sizeof(*(ol->ptr)) * ol->cap);
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


static bool TryParseToken(struct parserContext* pc, struct token* tok,
        enum tokenType type, bool discardNewlines) {

    struct token tmpTok;
    if (discardNewlines) {
        tmpTok = TokenNextDiscardNewlines(&(pc->tc));
    }
    else tmpTok = TokenNext(&(pc->tc));
    if (tmpTok.type != type) {
        TokenUnget(&(pc->tc));
        return false;
    }
    *tok = tmpTok;
    return true;
}


//errorMsg is autogenerated on NULL
static void ForceParseToken(struct parserContext* pc, struct token* tok,
        enum tokenType type, bool discardNewlines, char* errorMsg) {

    if (discardNewlines) {
        *tok = TokenNextDiscardNewlines(&(pc->tc));
    }
    else *tok = TokenNext(&(pc->tc));
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


//TODO make expr parser
static bool TryParseIntConstantExpr(struct parserContext* pc, int* intConst, bool discardNewlines) {
    struct token tok;
    if (TryParseToken(pc, &tok, TOKEN_INT, discardNewlines)) {
        //TODO atoi the int and set instConst to it
        (void)intConst;
        return true;
    }
    else if (TryParseToken(pc, &tok, TOKEN_IDENTIFIER, discardNewlines)) {
        //TODO fetch int constant from constant list
        (void)intConst;
        return true;
    }
    return false;
}


struct type ParseType(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, false, "expected type identifier");
    struct type t;
    if (!GetType(tok.str, &t, pc)) SyntaxErrorInvalidToken(tok, "unknown type");
    t.tok = tok;
    if (TryParseToken(pc, &tok, TOKEN_SQUAREBRACKET_OPEN, false)) {
        int arrLen = 0;
        bool ref = true;
        if (TryParseIntConstantExpr(pc, &arrLen, false)) ref = false;
        ForceParseToken(pc, &tok, TOKEN_SQUAREBRACKET_CLOSE, false, NULL);
        struct type arrT = ArrType(t, arrLen, ref);
        arrT.tok = t.tok;
        arrT.tok = TokenExtend(arrT.tok, tok);
        return arrT;
    }
    else if (t.bType == BASETYPE_STRUCT && TryParseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, false)) {
        t.ref = false;
        ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, false, NULL);
        t.tok = TokenExtend(t.tok, tok);
    }
    return t;
}


static void ParseStructTypedefMember(struct parserContext* pc, struct str structName, struct operandList* list) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, false, "expected variable identifier");
    if (operandListExists(*list, tok.str)) SyntaxErrorInvalidToken(tok, "duplicate member name");
    struct type t = ParseType(pc);
    if (!t.ref && StrEqual(structName, t.name)) {
        SyntaxErrorInvalidToken(t.tok, "structures may not instantiate themselves");
    }
    if (
            t.bType == BASETYPE_STRUCT && !t.ref && t.advanced != NULL &&
            typeListExists(((struct structTypeData*)t.advanced)->embeddedStructs, structName)) {

        SyntaxErrorInvalidToken(t.tok, "circular struct instantiation");
    }
    operandListAppend(list, OperandNew(tok.str, t));
}


struct operandList ParseStructTypedefMembers(struct parserContext* pc, struct str structName) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, false, NULL);
    struct operandList list = OperandListNew();
    if (TryParseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, true)) return list;
    ParseStructTypedefMember(pc, structName, &list);
    while (TryParseToken(pc, &tok, TOKEN_COMMA, false)) {
        TokenDiscardNewlines(&(pc->tc));
        ParseStructTypedefMember(pc, structName, &list);
    }
    ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, true, "expected \",\" or \"}\"");
    return list;
}


struct type ParseStructTypedef(struct parserContext* pc, struct str structName) {
    TokenDiscardNewlines(&(pc->tc));
    struct operandList members = ParseStructTypedefMembers(pc, structName);
    return StructType(members);
}


static void ParseVocabTypedefMember(struct parserContext* pc, struct strList* list) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, true, "expected word");
    if (StrListExists(list, tok.str)) SyntaxErrorInvalidToken(tok, "duplicate vocabulary word");
    StrListAppend(list, tok.str);
}


struct strList ParseVocabTypedefMembers(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_OPEN, false, NULL);
    struct strList list = StrListNew();
    ParseVocabTypedefMember(pc, &list);
    while (TryParseToken(pc, &tok, TOKEN_COMMA, false)) {
        TokenDiscardNewlines(&(pc->tc));
        ParseVocabTypedefMember(pc, &list);
    }
    ForceParseToken(pc, &tok, TOKEN_CURLYBRACKET_CLOSE, true, "expected \",\" or \"}\"");
    return list;
}


struct type ParseVocabTypedef(struct parserContext* pc) {
    struct strList words = ParseVocabTypedefMembers(pc);
    return VocabType(words);
}


struct type ParseFuncArgType(struct parserContext* pc) {
    bool mut = false;
    struct token tok;
    if (TryParseToken(pc, &tok, TOKEN_MUT, false)) mut = true;
    struct type t = ParseType(pc);
    if (t.bType == BASETYPE_ARRAY && !t.ref) {
        SyntaxErrorInvalidToken(t.tok, "function argument arrays must be references");
    }
    else if (t.bType == BASETYPE_STRUCT && !t.ref) {
        SyntaxErrorInvalidToken(t.tok, "function argument structs must be references");
    }

    if (mut) {
        TypeSetMutTrue(&t);
        if (t.bType != BASETYPE_ARRAY && t.bType != BASETYPE_STRUCT) {
            SyntaxErrorInvalidToken(t.tok, "only arrays or structures may be declared mutable in function args");
        }
    }

    return t;
}


struct typeList ParseFuncTypedefArgs(struct parserContext* pc) {
    struct typeList list = TypeListNew();
    struct token tok;

    ForceParseToken(pc, &tok, TOKEN_PAREN_OPEN, true, NULL);
    if (TryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true)) return list;
    TypeListAppend(&list, ParseFuncArgType(pc));

    while (TryParseToken(pc, &tok, TOKEN_COMMA, false)) {
        TokenDiscardNewlines(&(pc->tc));
        TypeListAppend(&list, ParseFuncArgType(pc));
    }

    ForceParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true, "expected \",\" or \")\"");
    return list;
}


struct typeList ParseFuncTypedefRets(struct parserContext* pc) {
    struct typeList list = TypeListNew();
    struct token tok;

    ForceParseToken(pc, &tok, TOKEN_PAREN_OPEN, true, NULL);
    if (TryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true)) return list;
    TypeListAppend(&list, ParseType(pc));

    while (TryParseToken(pc, &tok, TOKEN_COMMA, false)) {
        TokenDiscardNewlines(&(pc->tc));
        TypeListAppend(&list, ParseType(pc));
    }

    ForceParseToken(pc, &tok, TOKEN_PAREN_CLOSE, true, "expected \",\" or \")\"");
    return list;
}


struct type ParseFuncTypedef(struct parserContext* pc) {
    struct typeList args = ParseFuncTypedefArgs(pc);
    struct typeList rets = ParseFuncTypedefRets(pc);
    return FuncType(args, rets);
}


static struct type ParseTypedefSwitch(struct parserContext* pc, struct str typeName) {
    struct token tok = TokenNext(&(pc->tc));
    switch (tok.type) {
        case TOKEN_IDENTIFIER: TokenUnget(&(pc->tc)); return ParseType(pc);
        case TOKEN_STRUCT: return ParseStructTypedef(pc, typeName);
        case TOKEN_VOCAB: return ParseVocabTypedef(pc);
        case TOKEN_FUNC: return ParseFuncTypedef(pc);
        default: SyntaxErrorInvalidToken(tok, "not a type"); exit(EXIT_FAILURE);
    }
}


void ParseTypedef(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, false, "expected type name");
    struct type oldType = ParseTypedefSwitch(pc, tok.str);
    struct type newType = AliasType(oldType, tok.str);
    UpdateType(pc, newType);
    ForceParseToken(pc, &tok, TOKEN_NEWLINE, false, NULL);
}


void ParseGlobalLevel(struct parserContext* pc) {
    struct token tok = TokenNextDiscardNewlines(&(pc->tc));
    switch (tok.type) {
        case TOKEN_TYPE: ParseTypedef(pc); break; //typedefs are reparsed; this is defined behaviour
        case TOKEN_FUNC: break; //func headers are reparsed; this is defined behaviour
        default: //SyntaxErrorInvalidToken(tok, NULL); 
    }
}


static struct type TypePlaceholder(struct token name, enum baseType bType) {
    struct type t;
    t.ref = true;
    t.name = name.str;
    t.bType = bType;
    t.advanced = NULL;
    t.tok = name;
    return t;
}


void ParseTypePlaceholders(struct parserContext* pc) {
    struct token tok;
    while ((tok = TokenNext(&(pc->tc))).type != TOKEN_EOF) {
        if (tok.type == TOKEN_TYPE) {
            struct token nameTok;
            ForceParseToken(pc, &nameTok, TOKEN_IDENTIFIER, false, "expected type name");
            tok = TokenNext(&(pc->tc));
            switch (tok.type) {
                case TOKEN_IDENTIFIER:
                    if (TryParseToken(pc, &tok, TOKEN_SQUAREBRACKET_OPEN, false)) {
                        AddType(TypePlaceholder(nameTok, BASETYPE_ARRAY), pc);
                    }
                    struct type type;
                    if (!GetType(tok.str, &type, pc)) SyntaxErrorInvalidToken(tok, "invalid type");
                    else AddType(TypePlaceholder(nameTok, type.bType), pc);
                    break;

                case TOKEN_FUNC: AddType(TypePlaceholder(nameTok, BASETYPE_FUNC), pc); break;
                case TOKEN_STRUCT: AddType(TypePlaceholder(nameTok, BASETYPE_STRUCT), pc); break;
                case TOKEN_VOCAB: AddType(TypePlaceholder(nameTok, BASETYPE_VOCAB), pc); break;
                default: TokenUnget(&(pc->tc)); SyntaxErrorInvalidToken(tok, "invalid type");
            }
        }
    }
}


struct operand ParseVarDeclaration(struct parserContext* pc) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, false, "expected variable name");
    return OperandNew(tok.str, ParseType(pc));
}


void ParseFuncArgVarDeclaration(struct parserContext* pc, struct operandList* args) {
    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_IDENTIFIER, false, "expected variable name");
    if (isPublic(tok.str)) SyntaxErrorInvalidToken(tok, "local variables may not be capitalized");
    if (operandListExists(*args, tok.str)) SyntaxErrorInvalidToken(tok, "duplicate argument name");
    operandListAppend(args, OperandNew(tok.str, ParseFuncArgType(pc)));
}


struct operandList ParseFuncArgs(struct parserContext* pc) {
    struct operandList args = OperandListNew();

    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_PAREN_OPEN, false, NULL);
    if (TryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, false)) return args;

    ParseFuncArgVarDeclaration(pc, &args);
    while (!TryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, false)) {
        ForceParseToken(pc, &tok, TOKEN_COMMA, false, "expected \",\" or \")\"");
        ParseFuncArgVarDeclaration(pc, &args);
    }
    return args;
}


struct typeList ParseFuncRets(struct parserContext* pc) {
    struct typeList rets = TypeListNew();

    struct token tok;
    ForceParseToken(pc, &tok, TOKEN_PAREN_OPEN, false, NULL);
    if (TryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, false)) return rets;

    TypeListAppend(&rets, ParseType(pc));
    while (!TryParseToken(pc, &tok, TOKEN_PAREN_CLOSE, false)) {
        ForceParseToken(pc, &tok, TOKEN_COMMA, false, "expected \",\" or \")\"");
        TypeListAppend(&rets, ParseType(pc));
    }
    return rets;
}


struct type FuncTypeFromOpArgsAndRets(struct operandList args, struct typeList rets) {
    struct typeList argTypes = TypeListNew();
    struct typeList retTypes = TypeListNew();
    for (int i = 0; i < args.len; i++) {
        TypeListAppend(&argTypes, args.ptr[i].type);
    }
    for (int i = 0; i < rets.len; i++) {
        TypeListAppend(&retTypes, rets.ptr[i]);
    }
    return FuncType(argTypes, retTypes);
}


static struct operand ParseFuncHeader(struct parserContext* pc) {
    struct token nameTok;
    ForceParseToken(pc, &nameTok, TOKEN_IDENTIFIER, false, "expected function name");
    struct operandList args = ParseFuncArgs(pc);
    struct typeList rets = ParseFuncRets(pc);
    struct operand funcOp = OperandNew(nameTok.str, FuncTypeFromOpArgsAndRets(args, rets));
    funcOp.args = args;
    funcOp.rets = rets;
    return funcOp;
}


void ParseCompleteTypesAndFuncHeaders(struct parserContext* pc) {
    struct token tok;
    while ((tok = TokenNext(&(pc->tc))).type != TOKEN_EOF) {
        if (tok.type == TOKEN_TYPE) ParseTypedef(pc);
        else if (tok.type == TOKEN_FUNC) {
            ParseFuncHeader(pc);
        }
    }
}


struct parserContext parserContextNew(char* fileName) {
    struct parserContext pc;
    pc.tc = TokenContextNew(fileName);
    pc.publTypes = TypeListNew();
    pc.privTypes = TypeListNew();
    pc.publOps = OperandListNew();
    pc.privOps = OperandListNew();
    AppendBaseTypes(&(pc.privTypes));
    return pc;
}


void ParseFile(char* fileName) {
    struct parserContext pc = parserContextNew(fileName);
    struct token tok;
    ParseTypePlaceholders(&pc);
    TokenRestart(&(pc.tc));
    ParseCompleteTypesAndFuncHeaders(&pc);
    TokenRestart(&(pc.tc));
    while (!TryParseToken(&pc, &tok, TOKEN_EOF, true)) {
        ParseGlobalLevel(&pc);
    }
}
