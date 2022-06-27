#ifndef __ASTDEFINITION__
#define __ASTDEFINITION__

#include <memory>
#include <string>
#include <stack>
#include <iostream>
#include <variant>
#include <vector>
#include "assert.h"
#include "koopa.h"
#include "irdefinition.h"
#include "symboltable.h"

using namespace std;

extern KoopaBlock *cur_block;

class BaseAST
{
public:
    virtual ~BaseAST() = default;
    // virtual void Dump() const = 0;
    // virtual void synRawProgram(koopa_raw_program_t &p){};

    // virtual unique_ptr<KoopaBase> SynIR() const { return nullptr; };
    // static int sym_cnt;
};

class CompUnitAST;
class FuncDefAST;
class FuncTypeAST;
class StmtAST;
class BlockAST;
class BlockItemAST;
class NumberAST;
class ExpAST;
class PrimaryExpAST;
class UnaryExpAST;
class UnaryOpAST;
class MulExpAST;
class AddExpAST;
class RelExpAST;
class EqExpAST;
class LAndExpAST;
class LOrExpAST;
class DeclAST;
class ConstDeclAST;
class ConstDefAST;
class ConstInitValAST;
class LValAST;
class ConstExpAST;
class VarDeclAST;
class VarDefAST;
class InitValAST;
class FuncFParamAST;
class DeclPFuncDefAST;

// CompUnit 是 BaseAST
// CompUnit ::= [CompUnit] (Decl | FuncDef);
class CompUnitAST : public BaseAST
{
public:
    // 用智能指针管理对象
    vector<unique_ptr<FuncDefAST>> func_def_list;
    vector<unique_ptr<DeclAST>> decl_list;
    vector<unique_ptr<DeclPFuncDefAST>> list;
    void Dump() const;
    unique_ptr<KoopaProgram> SynIR() const;
    // void synRawProgram(koopa_raw_program_t &p) ;
};

// Decl | FuncDef
class DeclPFuncDefAST
{
public:
    int prod_id;
    unique_ptr<FuncDefAST> func_def;
    unique_ptr<DeclAST> decl;
};

// FuncDef 也是 BaseAST
// FuncDef     ::= FuncType IDENT "(" [FuncFParams] ")" Block;
class FuncDefAST : public BaseAST
{
public:
    unique_ptr<FuncTypeAST> func_type;
    string ident;
    vector<unique_ptr<FuncFParamAST>> func_f_params;
    unique_ptr<BlockAST> block;
    void Dump() const;
    KoopaFunDef *SynIR() const;
    // koopa_raw_function_t synRawFunction();
};

// FuncType    ::= "void" | "int";
class FuncTypeAST : public BaseAST
{
public:
    string ty;
    void Dump() const;
    // koopa_raw_type_t synRawType();
};

// Block         ::= "{" {BlockItem} "}";
class BlockAST : public BaseAST
{
public:
    vector<unique_ptr<BlockItemAST>> block_item_list;
    // unique_ptr<StmtAST> stmt;
    void Dump() const;
    void SynIR() const;

    // koopa_raw_basic_block_t synBasicBlock();
};

// BlockItem     ::= Decl | Stmt;
class BlockItemAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<DeclAST> decl;
    unique_ptr<StmtAST> stmt;
    void SynIR() const;
};

class NumberAST : public BaseAST
{
public:
    int int_const;
    void Dump() const;
    int SynIR() const;
    // koopa_raw_value_t synValue();
};

// Stmt ::= LVal "=" Exp ";"
//          | "return" [Exp] ";";
//          | [Exp] ";"
//          | Block
//          | "if" "(" Exp ")" Stmt ["else" Stmt]
//          | "while" "(" Exp ")" Stmt
//          | "break" ";"
//          | "continue" ";"
class StmtAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<LValAST> lval;
    unique_ptr<ExpAST> exp;
    unique_ptr<BlockAST> block;
    unique_ptr<StmtAST> t_stmt;
    unique_ptr<StmtAST> f_stmt;
    void Dump() const;
    void SynIR() const;
    // koopa_raw_value_t synValue();
};

// Exp :: = UnaryExp;
class ExpAST : public BaseAST
{
public:
    unique_ptr<LOrExpAST> lor_exp;
    void Dump() const;
    KoopaVal *SynIR() const;
    int eval() const;
};

// PrimaryExp    ::= "(" Exp ")" | Number | LVal;
class PrimaryExpAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<ExpAST> exp;
    unique_ptr<NumberAST> number;
    unique_ptr<LValAST> lval;

    void Dump() const;
    KoopaVal *SynIR() const;
    int eval() const;
};

// UnaryExp    ::= PrimaryExp | UnaryOp UnaryExp | IDENT "(" [FuncRParams] ")";
class UnaryExpAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<PrimaryExpAST> primary_exp;

    unique_ptr<UnaryOpAST> unary_op;
    unique_ptr<UnaryExpAST> unary_exp;

    string ident;
    vector<unique_ptr<ExpAST>> func_r_params;

    void Dump() const;
    KoopaVal *SynIR() const;
    int eval() const;
};

// UnaryOp     ::= "+" | "-" | "!";
class UnaryOpAST : public BaseAST
{
public:
    int prod_id;
    void Dump() const;
    KoopaStmt *SynIR(KoopaVal *val) const;
    int eval() const;
};
// MulExp      ::= UnaryExp | MulExp ("*" | "/" | "%") UnaryExp;
class MulExpAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<UnaryExpAST> unary_exp;
    unique_ptr<MulExpAST> mul_exp;
    string op;
    void Dump() const;
    KoopaVal *SynIR() const;
    int eval() const;
};

// AddExp      ::= MulExp | AddExp ("+" | "-") MulExp;
class AddExpAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<AddExpAST> add_exp;
    unique_ptr<MulExpAST> mul_exp;
    string op;
    void Dump() const;
    KoopaVal *SynIR() const;
    int eval() const;
};

// RelExp      ::= AddExp | RelExp ("<" | ">" | "<=" | ">=") AddExp;
class RelExpAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<AddExpAST> add_exp;
    unique_ptr<RelExpAST> rel_exp;
    string op;
    void Dump() const;
    KoopaVal *SynIR() const;
    int eval() const;
};

// EqExp       ::= RelExp | EqExp ("==" | "!=") RelExp;
class EqExpAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<EqExpAST> eq_exp;
    unique_ptr<RelExpAST> rel_exp;
    string op;
    void Dump() const;
    KoopaVal *SynIR() const;
    int eval() const;
};

//  LAndExp     ::= EqExp | LAndExp "&&" EqExp;
class LAndExpAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<EqExpAST> eq_exp;
    unique_ptr<LAndExpAST> land_exp;
    string op;
    void Dump() const;
    // KoopaVal *SynIR() const;
    int eval() const;
    KoopaVal *SynIRshortCircuit() const;
};

// LOrExp      ::= LAndExp | LOrExp "||" LAndExp;
class LOrExpAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<LOrExpAST> lor_exp;
    unique_ptr<LAndExpAST> land_exp;
    string op;
    void Dump() const;
    // KoopaVal *SynIR() const;
    int eval() const;
    KoopaVal *SynIRshortCircuit() const;
};

// Decl          ::= ConstDecl | VarDecl;
class DeclAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<ConstDeclAST> const_decl;
    unique_ptr<VarDeclAST> var_decl;
    void SynIR() const;
    void SynGlobalDecl() const;
};
// ConstDecl     ::= "const" BType ConstDef {"," ConstDef} ";";
class ConstDeclAST : public BaseAST
{
public:
    string b_type;
    vector<unique_ptr<ConstDefAST>> const_def_list;
    void SynIR() const;
    void SynGlobalDecl() const;
};

// ConstDef      ::= IDENT {"[" ConstExp "]"} "=" ConstInitVal;
class ConstDefAST : public BaseAST
{
public:
    string ident;
    vector<unique_ptr<ConstExpAST>> const_exps;
    unique_ptr<ConstInitValAST> const_init_val;
    void SynIR() const;
    void SynGlobalDecl() const;
};

// ConstInitVal  ::= ConstExp | "{" [ConstInitVal {"," ConstInitVal}] "}";
class ConstInitValAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<ConstExpAST> const_exp;
    vector<unique_ptr<ConstInitValAST>> const_init_vals;
    int eval() const;
};

// LVal          ::= IDENT {"[" Exp "]"};
class LValAST : public BaseAST
{
public:
    string ident;
    vector<unique_ptr<ExpAST>> exps;
    int eval() const;
    void SynIRStore(KoopaVal *val) const;
    KoopaVal *SynIRVal() const;
};

// ConstExp      ::= Exp;
class ConstExpAST : public BaseAST
{
public:
    unique_ptr<ExpAST> exp;
    int eval() const;
};

// VarDecl       ::= BType VarDef {"," VarDef} ";";
class VarDeclAST : public BaseAST
{
public:
    string b_type;
    vector<unique_ptr<VarDefAST>> var_def_list;
    void SynIR() const;
    void SynGlobalDecl() const;
};

// VarDef        ::= IDENT {"[" ConstExp "]"}
//                 | IDENT {"[" ConstExp "]"} "=" InitVal;
class VarDefAST : public BaseAST
{
public:
    int prod_id;
    string ident;
    unique_ptr<InitValAST> init_val;
    vector<unique_ptr<ConstExpAST>> const_exps;
    void SynIR() const;
    void SynGlobalDecl() const;
};

// InitVal       ::= Exp | "{" [InitVal {"," InitVal}] "}";
class InitValAST : public BaseAST
{
public:
    int prod_id;
    unique_ptr<ExpAST> exp;
    vector<unique_ptr<InitValAST>> init_vals;
    KoopaVal *SynIR() const;
};

// FuncFParam    ::= BType IDENT
//                 | BType IDENT "[" "]" {"[" ConstExp "]"};
class FuncFParamAST : public BaseAST
{
public:
    int prod_id;
    string b_type;
    string ident;
    vector<unique_ptr<ConstExpAST>> const_exps;
};

#endif