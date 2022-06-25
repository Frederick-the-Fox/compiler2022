#ifndef __IRDEFINITION__
#define __IRDEFINITION__

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include "assert.h"

using namespace std;

class KoopaBase;
class KoopaFunDef;
class KoopaFunBody;
class KoopaStmt;
class KoopaBlock;
class KoopaSymbolDef;
class KoopaBinaryExpr;
class KoopaVal;
class KoopaEndStmt;
class KoopaReturn;
class KoopaMemoryDeclaration;
class KoopaType;
class KoopaStore;
class KoopaLoad;
class KoopaBranch;
class KoopaJump;
class KoopaFunParams;
class KoopaFunCall;
class KoopaGlobalSymbolDef;
class KoopaGlobalMemoryDeclaration;
class KoopaInitializer;
class KoopaArrayType;
class KoopaGetElementPointer;
class KoopaAggregate;
class KoopaPointerType;
class KoopaGetPointer;

class KoopaBase
{
public:
    virtual ~KoopaBase() = default;
    // virtual string Dump(string &s) const { assert(false); };
};

class KoopaProgram : public KoopaBase
{
public:
    vector<unique_ptr<KoopaGlobalSymbolDef>> global_symbol_def_list;
    vector<unique_ptr<KoopaFunDef>> fun_def_list;
    void Dump(string &s) const;
};

// FunDef ::= "fun" SYMBOL "(" [FunParams] ")" [":" Type] "{" FunBody "}";
class KoopaFunDef : public KoopaBase
{
public:
    string symbol;
    unique_ptr<KoopaType> type;
    unique_ptr<KoopaFunBody> fun_body;
    unique_ptr<KoopaFunParams> fun_params;
    void Dump(string &s) const;
};

// FunBody ::= {Block};
class KoopaFunBody : public KoopaBase
{
public:
    vector<unique_ptr<KoopaBlock>> block_list;
    void Dump(string &s) const;
};

// class KoopaFunc : public KoopaBase
// {
// public:
//     vector<unique_ptr<KoopaBlock>> block_list;
//     string func_name;
//     string ret_type;
//     void Dump(string &s) const;
// };

typedef enum
{
    KP_ST_SYMBOL_DEF,
    KP_ST_STORE,
    KP_ST_FUN_CALL,
} KoopaStmtTag;

// Statement ::= SymbolDef | Store | FunCall;
class KoopaStmt : public KoopaBase
{
public:
    KoopaStmtTag tag;
    unique_ptr<KoopaSymbolDef> symbol_def;
    unique_ptr<KoopaStore> store;
    unique_ptr<KoopaFunCall> fun_call;

    KoopaStmt(KoopaStmtTag _tag) { tag = _tag; }
    KoopaStmt(KoopaSymbolDef *_symbol_def)
    {
        symbol_def = unique_ptr<KoopaSymbolDef>(_symbol_def);
        tag = KP_ST_SYMBOL_DEF;
    }
    KoopaStmt(KoopaStore *_store)
    {
        store = unique_ptr<KoopaStore>(_store);
        tag = KP_ST_STORE;
    }
    KoopaStmt(KoopaFunCall *_fun_call)
    {
        fun_call = unique_ptr<KoopaFunCall>(_fun_call);
        tag = KP_ST_FUN_CALL;
    }
    void Dump(string &s) const;
};

class KoopaBlock : public KoopaBase
{
public:
    KoopaBlock()
    {
        end_stmt = nullptr;
    }
    vector<unique_ptr<KoopaStmt>> stmt_list;
    string block_name;
    unique_ptr<KoopaEndStmt> end_stmt;
    void Dump(string &s) const;
};

typedef enum
{
    KP_SD_MEMORY_DECLARATION,
    KP_SD_LOAD,
    KP_SD_GET_POINTER,
    KP_SD_BINARY_EXPR,
    KP_SD_FUN_CALL,
    KP_SD_GET_ELEMENT_POINTER,
} KoopaSymbolDefTag;

// SymbolDef ::= SYMBOL "=" (MemoryDeclaration | Load | GetPointer | BinaryExpr | FunCall | GetElementPointer);
class KoopaSymbolDef : public KoopaBase
{
public:
    string symbol;
    KoopaSymbolDefTag tag;
    unique_ptr<KoopaBinaryExpr> binary_expr;
    unique_ptr<KoopaMemoryDeclaration> memory_declaration;
    unique_ptr<KoopaLoad> load;
    unique_ptr<KoopaFunCall> fun_call;
    unique_ptr<KoopaGetElementPointer> get_element_pointer;
    unique_ptr<KoopaGetPointer> get_pointer;
    KoopaSymbolDef(KoopaSymbolDefTag _tag, string _symbol) : symbol(_symbol), tag(_tag) {}
    KoopaSymbolDef(string _symbol, KoopaMemoryDeclaration *_memory_declaration) : symbol(_symbol)
    {
        tag = KP_SD_MEMORY_DECLARATION;
        memory_declaration = unique_ptr<KoopaMemoryDeclaration>(_memory_declaration);
    }
    KoopaSymbolDef(string _symbol, KoopaLoad *_load) : symbol(_symbol)
    {
        tag = KP_SD_LOAD;
        load = unique_ptr<KoopaLoad>(_load);
    }
    KoopaSymbolDef(string _symbol, KoopaFunCall *_fun_call) : symbol(_symbol)
    {
        tag = KP_SD_FUN_CALL;
        fun_call = unique_ptr<KoopaFunCall>(_fun_call);
    }
    KoopaSymbolDef(string _symbol, KoopaGetElementPointer *_get_element_pointer) : symbol(_symbol)
    {
        tag = KP_SD_GET_ELEMENT_POINTER;
        get_element_pointer = unique_ptr<KoopaGetElementPointer>(_get_element_pointer);
    }
    KoopaSymbolDef(string _symbol, KoopaGetPointer *_get_pointer) : symbol(_symbol)
    {
        tag = KP_SD_GET_POINTER;
        get_pointer = unique_ptr<KoopaGetPointer>(_get_pointer);
    }
    void Dump(string &s) const;
};

class KoopaBinaryExpr : public KoopaBase
{
public:
    string binary_op;
    unique_ptr<KoopaVal> val0, val1;
    KoopaBinaryExpr(string _binary_op, KoopaVal *_val0, KoopaVal *_val1)
    {
        binary_op = _binary_op;
        val0 = unique_ptr<KoopaVal>(_val0);
        val1 = unique_ptr<KoopaVal>(_val1);
    }
    void Dump(string &s) const;
};

typedef enum
{
    KP_VAL_INT,
    KP_VAL_SYMBOL,
    KP_VAL_UNDEF,
} KoopaValTag;

class KoopaVal : public KoopaBase
{
public:
    KoopaValTag tag;
    int data;
    string symbol;
    KoopaVal(int _data)
    {
        tag = KP_VAL_INT;
        data = _data;
    }
    KoopaVal(string _symbol)
    {
        tag = KP_VAL_SYMBOL;
        symbol = _symbol;
    }
    void Dump(string &s) const;
};

typedef enum
{
    KP_EST_BRANCH,
    KP_EST_JUMP,
    KP_EST_RETURN,
} KoopaEndStmtTag;

// EndStatement ::= Branch | Jump | Return;
class KoopaEndStmt : public KoopaBase
{
public:
    KoopaEndStmtTag tag;
    unique_ptr<KoopaReturn> Return;
    unique_ptr<KoopaJump> jump;
    unique_ptr<KoopaBranch> branch;
    KoopaEndStmt(KoopaReturn *_Return)
    {
        tag = KP_EST_RETURN;
        Return = unique_ptr<KoopaReturn>(_Return);
    }
    KoopaEndStmt(KoopaJump *_jump)
    {
        tag = KP_EST_JUMP;
        jump = unique_ptr<KoopaJump>(_jump);
    }
    KoopaEndStmt(KoopaBranch *_branch)
    {
        tag = KP_EST_BRANCH;
        branch = unique_ptr<KoopaBranch>(_branch);
    }
    void Dump(string &s) const;
};

// Return ::= "ret" [Value];
class KoopaReturn : public KoopaBase
{
public:
    unique_ptr<KoopaVal> val;
    KoopaReturn() { val = nullptr; }
    KoopaReturn(KoopaVal *_val) { val = unique_ptr<KoopaVal>(_val); }
    void Dump(string &s) const;
};

// Branch ::= "br" Value "," SYMBOL [BlockArgList] "," SYMBOL [BlockArgList];
class KoopaBranch : public KoopaBase
{
public:
    unique_ptr<KoopaVal> val;
    string symbol0, symbol1;
    KoopaBranch(KoopaVal *_val, string _symbol0, string _symbol1)
    {
        val = unique_ptr<KoopaVal>(_val);
        symbol0 = _symbol0;
        symbol1 = _symbol1;
    }
    void Dump(string &s) const;
};

// Jump ::= "jump" SYMBOL [BlockArgList];
class KoopaJump : public KoopaBase
{
public:
    string symbol;
    KoopaJump(string _symbol) : symbol(_symbol) {}
    void Dump(string &s) const;
};

// MemoryDeclaration ::= "alloc" Type;
class KoopaMemoryDeclaration
{
public:
    void Dump(string &s) const;
    KoopaMemoryDeclaration(KoopaType *_ty) { ty = unique_ptr<KoopaType>(_ty); }
    unique_ptr<KoopaType> ty;
};

typedef enum
{
    KP_TY_I32,
    KP_TY_ARRAY_TYPE,
    KP_TY_POINTER_TYPE,
    KP_TY_FUN_TYPE,
} KoopaTypeTag;

// Type ::= "i32" | ArrayType | PointerType | FunType;
class KoopaType
{
public:
    KoopaTypeTag tag;
    unique_ptr<KoopaArrayType> array_type;
    unique_ptr<KoopaPointerType> pointer_type;
    KoopaType(KoopaTypeTag _tag) { tag = _tag; }
    KoopaType(KoopaArrayType *_array_type)
    {
        tag = KP_TY_ARRAY_TYPE;
        array_type = unique_ptr<KoopaArrayType>(_array_type);
    }
    KoopaType(KoopaPointerType *_pointer_type)
    {
        tag = KP_TY_POINTER_TYPE;
        pointer_type = unique_ptr<KoopaPointerType>(_pointer_type);
    }
    void Dump(string &s) const;
};
// Store ::= "store" (Value | Initializer) "," SYMBOL;
class KoopaStore
{
public:
    unique_ptr<KoopaVal> val;
    string symbol;
    KoopaStore(KoopaVal *_val, string _symbol) : symbol(_symbol) { val = unique_ptr<KoopaVal>(_val); }
    void Dump(string &s) const;
};
// Load ::= "load" SYMBOL;
class KoopaLoad
{
public:
    string symbol;
    KoopaLoad(string _symbol) { symbol = _symbol; }
    void Dump(string &s) const;
};

struct FunParam
{
    string symbol;
    unique_ptr<KoopaType> type;
    FunParam(string _symbol, KoopaType *_type) : symbol(_symbol)
    {
        type = unique_ptr<KoopaType>(_type);
    }
    void Dump(string &s) const;
};

// FunParams ::= SYMBOL ":" Type {"," SYMBOL ":" Type};
class KoopaFunParams
{
public:
    vector<FunParam> list;
    void Dump(string &s) const;
};

// FunCall ::= "call" SYMBOL "(" [Value {"," Value}] ")";
class KoopaFunCall
{
public:
    string symbol;
    vector<unique_ptr<KoopaVal>> value_list;
    KoopaFunCall(string _symbol) : symbol(_symbol) {}
    void Dump(string &s) const;
};

// GlobalSymbolDef ::= "global" SYMBOL "=" GlobalMemoryDeclaration;
class KoopaGlobalSymbolDef
{
public:
    string symbol;
    unique_ptr<KoopaGlobalMemoryDeclaration> global_memory_declaration;
    KoopaGlobalSymbolDef(string _symbol, KoopaGlobalMemoryDeclaration *_global_memory_declaration) : symbol(_symbol)
    {
        global_memory_declaration = unique_ptr<KoopaGlobalMemoryDeclaration>(_global_memory_declaration);
    }

    void Dump(string &s) const;
};

// GlobalMemoryDeclaration ::= "alloc" Type "," Initializer;
class KoopaGlobalMemoryDeclaration
{
public:
    unique_ptr<KoopaType> type;
    unique_ptr<KoopaInitializer> initializer;
    KoopaGlobalMemoryDeclaration(KoopaType *_type, KoopaInitializer *_initializer)
    {
        type = unique_ptr<KoopaType>(_type);
        initializer = unique_ptr<KoopaInitializer>(_initializer);
    }
    void Dump(string &s) const;
};

typedef enum
{
    KP_INIT_INT,
    KP_INIT_UNDEF,
    KP_INIT_AGGREGATE,
    KP_INIT_ZEROINIT,
} KoopaInitializerTag;

// Initializer ::= INT | "undef" | Aggregate | "zeroinit";
class KoopaInitializer
{
public:
    KoopaInitializerTag tag;
    int data;
    unique_ptr<KoopaAggregate> aggregate;
    KoopaInitializer(int _data) : data(_data)
    {
        tag = KP_INIT_INT;
    }
    KoopaInitializer(KoopaAggregate *_aggregate)
    {
        tag = KP_INIT_AGGREGATE;
        aggregate = unique_ptr<KoopaAggregate>(_aggregate);
    }
    KoopaInitializer(KoopaInitializerTag _tag) : tag(_tag) {}
    void Dump(string &s) const;
};

// ArrayType ::= "[" Type "," INT "]";
class KoopaArrayType
{
public:
    unique_ptr<KoopaType> type;
    int data;
    KoopaArrayType(KoopaType *_type, int _data) : data(_data)
    {
        type = unique_ptr<KoopaType>(_type);
    }
    void Dump(string &s) const;
};

// GetElementPointer ::= "getelemptr" SYMBOL "," Value;
class KoopaGetElementPointer
{
public:
    string symbol;
    unique_ptr<KoopaVal> val;
    KoopaGetElementPointer(string _symbol, KoopaVal *_val) : symbol(_symbol)
    {
        val = unique_ptr<KoopaVal>(_val);
    }
    void Dump(string &s) const;
};

// GetPointer ::= "getptr" SYMBOL "," Value;
class KoopaGetPointer
{
public:
    string symbol;
    unique_ptr<KoopaVal> val;
    KoopaGetPointer(string _symbol, KoopaVal *_val) : symbol(_symbol)
    {
        val = unique_ptr<KoopaVal>(_val);
    }
    void Dump(string &s) const;
};

// Aggregate ::= "{" Initializer {"," Initializer} "}";
class KoopaAggregate
{
public:
    vector<unique_ptr<KoopaInitializer>> initializers;
    void Dump(string &s) const;
};

// PointerType ::= "*" Type;
class KoopaPointerType
{
public:
    unique_ptr<KoopaType> type;
    void Dump(string &s) const;
    KoopaPointerType(KoopaType *_type)
    {
        type = unique_ptr<KoopaType>(_type);
    }
};

#endif
