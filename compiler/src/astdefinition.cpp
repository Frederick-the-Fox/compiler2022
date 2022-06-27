#include "astdefinition.h"
#include "astaux.h"
#include "arrayaux.h"

using namespace std;

int if_cnt = 0;
int lor_cnt = 0;
int land_cnt = 0;
int while_cnt = 0;

KoopaFunDef *cur_fun = nullptr;
KoopaBlock *cur_block = nullptr;
KoopaProgram *prog = nullptr;

struct LoopInfo
{
    string entry, body, end;
    LoopInfo(string _entry, string _body, string _end) : entry(_entry), body(_body), end(_end) {}
};

stack<LoopInfo> Lstack;

void endCurBlock()
{
    assert(cur_block != nullptr);
    cur_fun->fun_body->block_list.push_back(unique_ptr<KoopaBlock>(cur_block));
    cur_block = nullptr;
    // clog << "endCurBlock" << endl;
}

void createNewBlock(string name)
{
    assert(cur_block == nullptr);
    // clog << "createNewBlock" << endl;
    cur_block = new KoopaBlock;
    cur_block->block_name = name;
}

void CompUnitAST::Dump() const
{

    cout << "CompUnitAST { ";
    // std::cout << "begin" << std::endl;
    // for (auto iter = func_def_list.begin(); iter != func_def_list.end(); iter++)
    //     (*iter)->Dump();
    //     std::cout << "mid" << std::endl;

    cout << " }";
}

unique_ptr<KoopaProgram> CompUnitAST::SynIR() const
{
    initST();
    initFTable();
    prog = new KoopaProgram;
    for (auto iter = list.begin(); iter != list.end(); iter++)
    {
        if ((*iter)->prod_id == 1)
        {
            (*iter)->decl->SynGlobalDecl();
        }
        else if ((*iter)->prod_id == 2)
        {
            prog->fun_def_list.push_back(unique_ptr<KoopaFunDef>((*iter)->func_def->SynIR()));
        }
        else
            assert(false);
    }
    quitCurST();
    return unique_ptr<KoopaProgram>(prog);
}

void FuncDefAST::Dump() const
{
    cout << "FuncDefAST { ";
    func_type->Dump();
    cout << ", " << ident << ", ";
    block->Dump();
    cout << " }";
}

KoopaFunDef *FuncDefAST::SynIR() const
{
    // clog << "FuncDefAST::SynIR() " << ident << endl;
    auto ret = new KoopaFunDef;
    cur_fun = ret;
    ret->symbol = "@" + ident;
    if (func_type->ty == "int")
        ret->type = make_unique<KoopaType>(KP_TY_I32);
    else
        ret->type = nullptr;
    assert(FTable.count(ident) == 0);
    FTable.insert({ident, FunInfo(func_type->ty)});

    ret->fun_body = make_unique<KoopaFunBody>();
    ret->fun_params = make_unique<KoopaFunParams>();
    createNewBlock("\%entry");
    buildNewST();

    for (auto iter = func_f_params.begin(); iter != func_f_params.end(); iter++)
    {
        auto par = (*iter).get();
        string tmps = "%" + par->ident; // temporary symbol
        string cons = "@" + par->ident; // with name
        if (par->prod_id == 1)          // is scala param
        {
            STInsertVar((*iter)->ident, tmps);
            ret->fun_params->list.push_back(FunParam(cons, new KoopaType(KP_TY_I32)));
            cur_block->stmt_list.push_back(make_symbol_def_alloc(tmps, new KoopaType(KP_TY_I32)));
            cur_block->stmt_list.push_back(make_store(new KoopaVal(cons), tmps));
        }
        else if (par->prod_id == 2) // is pointer param
        {
            vector<int> base;
            for (auto it = par->const_exps.begin(); it != par->const_exps.end(); it++)
                base.push_back((*it)->eval());

            ret->fun_params->list.push_back(FunParam(cons, get_pointer_type(base)));
            Pointer ptr(tmps, base);
            STInsertPointer((*iter)->ident, ptr);
            cur_block->stmt_list.push_back(make_symbol_def_alloc(tmps, get_pointer_type(base)));
            cur_block->stmt_list.push_back(make_store(new KoopaVal(cons), tmps));
        }
        else
            assert(false);
    }

    block->SynIR();

    if (cur_block != nullptr)
    {
        assert(cur_block->end_stmt == nullptr);
        if (ret->type == nullptr)
            cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaReturn());
        else
            cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaReturn(new KoopaVal(0)));
        endCurBlock();
    }
    quitCurST();

    // ret->fun_body->block_list.push_back(unique_ptr<KoopaBlock>(bb));
    // clog << "FuncDefAST::SynIR() finished." << endl;

    return ret;
}

void FuncTypeAST::Dump() const
{
    cout << "FuncTypeAST { ";
    cout << " int ";
    cout << " }";
}

void BlockAST::Dump() const
{
    cout << "BlockAST { ";
    cout << "{";
    // stmt->Dump();
    cout << "}";
    cout << " }";
}
void BlockAST::SynIR() const
{
    // clog << "BlockAST SynIR length: " << ' ' << block_item_list.size() << endl;
    buildNewST();
    for (auto iter = block_item_list.begin(); iter != block_item_list.end(); iter++)
    {
        (*iter)->SynIR();
        if (cur_block == nullptr) // is return
            break;
    }
    quitCurST();
}

void BlockItemAST::SynIR() const
{
    // clog << "BlockItemAST SynIR prod_id: " << prod_id << endl;
    if (prod_id == 1)
        decl->SynIR();
    else if (prod_id == 2)
        stmt->SynIR();
    else
        assert(false);
}

void NumberAST::Dump() const
{
    cout << "NumberAST { ";
    cout << int_const;
    cout << " }";
}

int NumberAST::SynIR() const
{
    return this->int_const;
}

void StmtAST::Dump() const
{
    cout << "StmtAST { ";
    cout << " return ";
    exp->Dump();
    cout << "; ";
    cout << " }";
}
void StmtAST::SynIR() const
{
    KoopaVal *val;
    string scnt, sym_then, sym_else, sym_end, tmp;
    switch (prod_id)
    {
    case 1:
        val = exp->SynIR();
        lval->SynIRStore(val);
        break;
    case 2:
        if (exp != nullptr)
        {
            val = exp->SynIR();
            cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaReturn(val));
        }
        else
            cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaReturn());
        endCurBlock();
        break;
    case 3:
        if (exp != nullptr)
            exp->SynIR();
        break;
    case 4:
        block->SynIR();
        break;
    case 5:
        if_cnt++;
        scnt = to_string(if_cnt);
        sym_then = "\%then_" + scnt, sym_else = "\%else_" + scnt, sym_end = "\%end_" + scnt;

        // val = exp->shortCircuit(sym_then, f_stmt == nullptr ? sym_end : sym_else);
        val = exp->SynIR();
        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaBranch(val, sym_then, f_stmt == nullptr ? sym_end : sym_else));
        endCurBlock();

        // %if:
        createNewBlock(sym_then);
        t_stmt->SynIR();
        if (cur_block != nullptr)
        {
            cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaJump(sym_end));
            endCurBlock();
        }

        //%else:
        if (f_stmt != nullptr)
        {
            createNewBlock(sym_else);
            f_stmt->SynIR();
            if (cur_block != nullptr)
            {
                cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaJump(sym_end));
                endCurBlock();
            }
        }

        //%end:
        createNewBlock(sym_end);
        break;

    case 6:
    {
        while_cnt++;
        tmp = to_string(while_cnt);
        LoopInfo l_info("%while" + tmp + "_entry", "%while" + tmp + "_body", "%while" + tmp + "_end");
        Lstack.push(l_info);

        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaJump(l_info.entry));
        endCurBlock();

        createNewBlock(l_info.entry);
        val = exp->SynIR();
        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaBranch(val, l_info.body, l_info.end));
        endCurBlock();

        createNewBlock(l_info.body);
        t_stmt->SynIR();
        if (cur_block != nullptr)
        {
            cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaJump(l_info.entry));
            endCurBlock();
        }

        Lstack.pop();
        createNewBlock(l_info.end);
    }
    break;

    case 7:
        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaJump(Lstack.top().end));
        endCurBlock();
        break;
    case 8:
        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaJump(Lstack.top().entry));
        endCurBlock();
        break;
    default:
        assert(false);
    }
}

void ExpAST::Dump() const
{
    cout << "ExpAST { ";
    lor_exp->Dump();
    cout << " }";
}

KoopaVal *ExpAST::SynIR() const
{
    return lor_exp->SynIRshortCircuit();
}

int ExpAST::eval() const
{
    return lor_exp->eval();
}

void PrimaryExpAST::Dump() const
{
    cout << "PrimaryExpAST { ";
    if (prod_id == 1)
    {
        cout << "( ";
        exp->Dump();
        cout << " )";
    }
    else if (prod_id == 2)
    {
        number->Dump();
    }
}
KoopaVal *PrimaryExpAST::SynIR() const
{
    KoopaVal *ret = nullptr;
    switch (prod_id)
    {
    case 1:
        ret = exp->SynIR();
        break;
    case 2:
        ret = new KoopaVal(number->SynIR());
        break;
    case 3:
        ret = lval->SynIRVal();
        break;
    default:
        assert(false);
    }
    return ret;
}

int PrimaryExpAST::eval() const
{
    switch (prod_id)
    {
    case 1:
        return exp->eval();
    case 2:
        return number->int_const;
    case 3:
        return lval->eval();
    default:
        assert(false);
    }
}

void UnaryExpAST::Dump() const
{
    cout << "UnaryExpAST{ ";
    if (prod_id == 1)
        primary_exp->Dump();
    else if (prod_id == 2)
    {
        unary_op->Dump();
        unary_exp->Dump();
    }
    else if (prod_id == 3)
    {
    }
    else
        assert(false);
    cout << " }";
}
KoopaVal *UnaryExpAST::SynIR() const
{
    KoopaVal *val;
    KoopaStmt *newstmt;
    KoopaVal *ret = nullptr;
    // clog << "UnaryExpAST::SynIR: " << static_cast<const void *>(this) << ' ' << prod_id << endl;
    if (prod_id == 1)
    {
        ret = primary_exp->SynIR();
    }
    else if (prod_id == 2)
    {
        val = unary_exp->SynIR();
        newstmt = unary_op->SynIR(val);
        if (newstmt != nullptr)
        {
            cur_block->stmt_list.push_back(unique_ptr<KoopaStmt>(newstmt));
            ret = new KoopaVal(newstmt->symbol_def->symbol);
        }
        else
            ret = val;
    }
    else if (prod_id == 3)
    {
        assert(FTable.count(ident) == 1);
        if (FTable[ident].type == "void")
        {
            auto fun_call = new KoopaFunCall("@" + ident);
            for (auto iter = func_r_params.begin(); iter != func_r_params.end(); iter++)
                fun_call->value_list.push_back(unique_ptr<KoopaVal>((*iter)->SynIR()));
            cur_block->stmt_list.push_back(make_unique<KoopaStmt>(fun_call));
            ret = nullptr;
        }
        else
        {
            string sym = get_new_sym();
            auto st = make_symbol_def_fun_call(sym, "@" + ident, func_r_params);
            cur_block->stmt_list.push_back(unique_ptr<KoopaStmt>(st));
            ret = new KoopaVal(sym);
        }
    }
    else
        assert(false);

    // clog << "UnaryExpAST::SynIR " << static_cast<const void *>(this) << " finished." << endl;
    return ret;
}

int UnaryExpAST::eval() const
{
    if (prod_id == 1)
        return primary_exp->eval();
    else if (prod_id == 2)
    {
        if (unary_op->prod_id == 1)
            return unary_exp->eval();
        else if (unary_op->prod_id == 2)
            return -unary_exp->eval();
        else if (unary_op->prod_id == 2)
            return !unary_exp->eval();
        else
            assert(false);
    }
    else
        assert(false);
}

void UnaryOpAST::Dump() const
{
    if (prod_id == 1)
        cout << "+";
    else if (prod_id == 2)
        cout << "-";
    else if (prod_id == 3)
        cout << "!";
}

// make a new symbol define and return the symbol
string make_new_symbol_def(string bi_op, KoopaVal *val0, KoopaVal *val1)
{
    string sym = get_new_sym();
    KoopaStmt *newstmt = new KoopaStmt(KP_ST_SYMBOL_DEF);
    newstmt->symbol_def = make_unique<KoopaSymbolDef>(KP_SD_BINARY_EXPR, sym);
    newstmt->symbol_def->binary_expr = make_unique<KoopaBinaryExpr>(bi_op, val0, val1);
    cur_block->stmt_list.push_back(unique_ptr<KoopaStmt>(newstmt));
    return newstmt->symbol_def->symbol;
}

KoopaStmt *UnaryOpAST::SynIR(KoopaVal *val) const
{
    KoopaStmt *ret;
    if (prod_id == 1) // +
        return nullptr;
    else if (prod_id == 2) // -
    {
        ret = new KoopaStmt(KP_ST_SYMBOL_DEF);
        ret->symbol_def = make_unique<KoopaSymbolDef>(KP_SD_BINARY_EXPR, get_new_sym());
        ret->symbol_def->binary_expr = make_unique<KoopaBinaryExpr>("sub", new KoopaVal(0), val);
    }
    else if (prod_id == 3) // !
    {
        ret = new KoopaStmt(KP_ST_SYMBOL_DEF);
        ret->symbol_def = make_unique<KoopaSymbolDef>(KP_SD_BINARY_EXPR, get_new_sym());
        ret->symbol_def->binary_expr = make_unique<KoopaBinaryExpr>("eq", new KoopaVal(0), val);
    }
    else
        assert(false);
    return ret;
}

void MulExpAST::Dump() const {}
KoopaVal *MulExpAST::SynIR() const
{
    KoopaVal *val0, *val1;
    KoopaVal *ret = nullptr;
    if (prod_id == 1)
        ret = unary_exp->SynIR();
    else if (prod_id == 2)
    {
        val0 = mul_exp->SynIR();
        val1 = unary_exp->SynIR();
        string bi_op = (op == "*" ? "mul" : (op == "/" ? "div" : "mod"));
        string sym = make_new_symbol_def(bi_op, val0, val1);
        ret = new KoopaVal(sym);
    }
    else
        assert(false);
    return ret;
}
int MulExpAST::eval() const
{
    if (prod_id == 1)
        return unary_exp->eval();
    else if (prod_id == 2)
    {
        if (op == "*")
            return mul_exp->eval() * unary_exp->eval();
        else if (op == "/")
            return mul_exp->eval() / unary_exp->eval();
        else if (op == "%")
            return mul_exp->eval() % unary_exp->eval();
        else
            assert(false);
    }
    else
        assert(false);
}

void AddExpAST::Dump() const {}
KoopaVal *AddExpAST::SynIR() const
{
    KoopaVal *val0, *val1;
    KoopaVal *ret = nullptr;
    if (prod_id == 1)
        ret = mul_exp->SynIR();
    else if (prod_id == 2)
    {
        val0 = add_exp->SynIR();
        val1 = mul_exp->SynIR();
        string bi_op = (op == "+" ? "add" : "sub");
        string sym = make_new_symbol_def(bi_op, val0, val1);
        ret = new KoopaVal(sym);
    }
    else
        assert(false);
    return ret;
}
int AddExpAST::eval() const
{
    if (prod_id == 1)
        return mul_exp->eval();
    else if (prod_id == 2)
    {
        if (op == "+")
            return add_exp->eval() + mul_exp->eval();
        else if (op == "-")
            return add_exp->eval() - mul_exp->eval();
        else
            assert(false);
    }
    else
        assert(false);
}
void RelExpAST::Dump() const {}
KoopaVal *RelExpAST::SynIR() const
{
    KoopaVal *val0, *val1;
    KoopaVal *ret = nullptr;
    if (prod_id == 1)
        ret = add_exp->SynIR();
    else if (prod_id == 2)
    {
        val0 = rel_exp->SynIR();
        val1 = add_exp->SynIR();
        string bi_op = (op == "<" ? "lt" : (op == ">" ? "gt" : (op == "<=" ? "le" : "ge")));
        string sym = make_new_symbol_def(bi_op, val0, val1);
        ret = new KoopaVal(sym);
    }
    else
        assert(false);
    return ret;
}
int RelExpAST::eval() const
{
    if (prod_id == 1)
        return add_exp->eval();
    else if (prod_id == 2)
    {
        if (op == "<")
            return rel_exp->eval() < add_exp->eval();
        else if (op == ">")
            return rel_exp->eval() > add_exp->eval();
        else if (op == "<=")
            return rel_exp->eval() <= add_exp->eval();
        else if (op == ">=")
            return rel_exp->eval() >= add_exp->eval();
        else
            assert(false);
    }
    else
        assert(false);
}

void EqExpAST::Dump() const {}
KoopaVal *EqExpAST::SynIR() const
{
    KoopaVal *val0, *val1;
    KoopaVal *ret = nullptr;
    if (prod_id == 1)
        ret = rel_exp->SynIR();
    else if (prod_id == 2)
    {
        val0 = eq_exp->SynIR();
        val1 = rel_exp->SynIR();
        string bi_op = (op == "==" ? "eq" : "ne");
        string sym = make_new_symbol_def(bi_op, val0, val1);
        ret = new KoopaVal(sym);
    }
    else
        assert(false);
    return ret;
}
int EqExpAST::eval() const
{
    if (prod_id == 1)
        return rel_exp->eval();
    else if (prod_id == 2)
    {
        if (op == "==")
            return eq_exp->eval() == rel_exp->eval();
        else if (op == "!=")
            return eq_exp->eval() != rel_exp->eval();
        else
            assert(false);
    }
    else
        assert(false);
}

void LAndExpAST::Dump() const {}
// KoopaVal *LAndExpAST::SynIR() const
// {
//     KoopaVal *val0, *val1;
//     KoopaVal *ret = nullptr;
//     if (prod_id == 1)
//         ret = eq_exp->SynIR();
//     else if (prod_id == 2)
//     {
//         val0 = land_exp->SynIR();
//         val1 = eq_exp->SynIR();
//         string sym0 = make_new_symbol_def("ne", val0, new KoopaVal(0));
//         string sym1 = make_new_symbol_def("ne", val1, new KoopaVal(0));
//         string sym2 = make_new_symbol_def("and", new KoopaVal(sym0), new KoopaVal(sym1));
//         ret = new KoopaVal(sym2);
//     }
//     else
//         assert(false);
//     return ret;
// }
int LAndExpAST::eval() const
{
    if (prod_id == 1)
        return eq_exp->eval();
    else if (prod_id == 2)
        return land_exp->eval() && eq_exp->eval();
    else
        assert(false);
}

void LOrExpAST::Dump() const {}
// KoopaVal *LOrExpAST::SynIR() const
// {
//     KoopaVal *val0, *val1;
//     KoopaVal *ret = nullptr;
//     if (prod_id == 1)
//         ret = land_exp->SynIR();
//     else if (prod_id == 2)
//     {
//         val0 = lor_exp->SynIR();
//         val1 = land_exp->SynIR();
//         string sym0 = make_new_symbol_def("ne", val0, new KoopaVal(0));
//         string sym1 = make_new_symbol_def("ne", val1, new KoopaVal(0));
//         string sym2 = make_new_symbol_def("or", new KoopaVal(sym0), new KoopaVal(sym1));
//         ret = new KoopaVal(sym2);
//     }
//     else
//         assert(false);
//     return ret;
// }

int LOrExpAST::eval() const
{
    if (prod_id == 1)
        return land_exp->eval();
    else if (prod_id == 2)
        return lor_exp->eval() || land_exp->eval();
    else
        assert(false);
}

void DeclAST::SynIR() const
{
    // clog << "DeclAST synIR" << endl;
    if (prod_id == 1)
        const_decl->SynIR();
    else if (prod_id == 2)
        var_decl->SynIR();
    else
        assert(false);
}

void DeclAST::SynGlobalDecl() const
{
    if (prod_id == 1)
        const_decl->SynGlobalDecl();
    else if (prod_id == 2)
        var_decl->SynGlobalDecl();
    else
        assert(false);
}

void ConstDeclAST::SynIR() const
{
    for (auto iter = const_def_list.begin(); iter != const_def_list.end(); iter++)
        (*iter)->SynIR();
}

void ConstDeclAST::SynGlobalDecl() const
{
    for (auto iter = const_def_list.begin(); iter != const_def_list.end(); iter++)
        (*iter)->SynGlobalDecl();
}

void ConstDefAST::SynIR() const
{
    if (const_exps.empty())
    {
        int eval = const_init_val->eval();
        // clog << "constdef insert: (" << ident << ", " << eval << ")" << endl;
        STInsertConst(ident, eval);
        return;
    }

    Array arr;
    arr.name = "@" + ident + "_" + to_string(STGetCnt());
    for (auto iter = const_exps.begin(); iter != const_exps.end(); iter++)
        arr.dim.push_back((*iter)->eval());
    STInsertArray(ident, arr);

    cur_block->stmt_list.push_back(make_symbol_def_alloc(arr.name, get_array_type(arr.dim)));

    init_local_const_array(arr, const_init_val.get());
}

void ConstDefAST::SynGlobalDecl() const
{
    if (const_exps.empty())
    {
        this->SynIR();
        return;
    }

    Array arr;
    arr.name = "@" + ident + "_" + to_string(STGetCnt());
    for (auto iter = const_exps.begin(); iter != const_exps.end(); iter++)
        arr.dim.push_back((*iter)->eval());
    STInsertArray(ident, arr);

    auto tmp = init_global_const_array(arr, const_init_val.get());
    auto p = new KoopaGlobalMemoryDeclaration(get_array_type(arr.dim), tmp);
    auto q = new KoopaGlobalSymbolDef(arr.name, p);
    prog->global_symbol_def_list.push_back(unique_ptr<KoopaGlobalSymbolDef>(q));
}

int ConstInitValAST::eval() const
{
    assert(prod_id == 1);
    return const_exp->eval();
}
int LValAST::eval() const
{
    assert(STExist(ident));
    // if (isGlobalVar(ident))
    // {
    //     assert(false);
    //     return STGetGlobalVarInitVal(ident);
    // }
    int p = STGetConst(ident);
    // assert(p->tag == KP_VAL_INT);
    return p;
}

int ConstExpAST::eval() const
{
    return exp->eval();
}

void VarDeclAST::SynIR() const
{
    for (auto iter = var_def_list.begin(); iter != var_def_list.end(); iter++)
        (*iter)->SynIR();
}

void VarDeclAST::SynGlobalDecl() const
{
    for (auto iter = var_def_list.begin(); iter != var_def_list.end(); iter++)
        (*iter)->SynGlobalDecl();
}

void VarDefAST::SynIR() const
{
    Array arr;
    arr.name = "@" + ident + "_" + to_string(STGetCnt());
    for (auto iter = const_exps.begin(); iter != const_exps.end(); iter++)
        arr.dim.push_back((*iter)->eval());
    STInsertArray(ident, arr);

    cur_block->stmt_list.push_back(make_symbol_def_alloc(arr.name, get_array_type(arr.dim)));
    if (prod_id == 2)

        init_local_var_array(arr, init_val.get());
}

void VarDefAST::SynGlobalDecl() const
{
    Array arr;
    arr.name = "@" + ident + "_" + to_string(STGetCnt());
    KoopaGlobalMemoryDeclaration *p;

    for (auto iter = const_exps.begin(); iter != const_exps.end(); iter++)
        arr.dim.push_back((*iter)->eval());
    STInsertArray(ident, arr);

    if (prod_id == 1)
        p = new KoopaGlobalMemoryDeclaration(get_array_type(arr.dim), new KoopaInitializer(KP_INIT_ZEROINIT));

    if (prod_id == 2)
    {
        auto tmp = init_global_var_array(arr, init_val.get());
        p = new KoopaGlobalMemoryDeclaration(get_array_type(arr.dim), tmp);
    }
    auto q = new KoopaGlobalSymbolDef(arr.name, p);
    prog->global_symbol_def_list.push_back(unique_ptr<KoopaGlobalSymbolDef>(q));
}

void LValAST::SynIRStore(KoopaVal *val) const
{
    assert(STExist(ident));
    auto p = STGet(ident);
    if (p.index() == 1)
    {
        auto arr = get<Array>(p);
        string res = array_access(arr, exps);
        cur_block->stmt_list.push_back(make_store(val, res));
    }
    else if (p.index() == 2)
    {
        auto ptr = get<Pointer>(p);
        string res = get_new_sym();
        cur_block->stmt_list.push_back(make_symbol_def_load(res, ptr.name));
        if (!exps.empty())
            res = pointer_access(res, ptr.base, exps);
        cur_block->stmt_list.push_back(make_store(val, res));
    }
    else
        assert(false);
}

KoopaVal *LValAST::SynIRVal() const
{
    assert(STExist(ident));

    auto p = STGet(ident);
    if (p.index() == 0) // is constant
        return new KoopaVal(get<Constant>(p).val);
    else if (p.index() == 1) // is variable or array
    {
        auto arr = get<Array>(p);
        string res = array_access(arr, exps);
        string sym = get_new_sym();

        if (arr.dim.size() == exps.size())
            cur_block->stmt_list.push_back(make_symbol_def_load(sym, res));
        else // partial dereference
            cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(sym, res, new KoopaVal(0)));

        return new KoopaVal(sym);
    }
    else if (p.index() == 2)
    {
        auto ptr = get<Pointer>(p);
        string sym = get_new_sym();
        cur_block->stmt_list.push_back(make_symbol_def_load(sym, ptr.name));

        int expsz = exps.size();
        if (expsz > 0)
        {
            string res = pointer_access(sym, ptr.base, exps);
            sym = get_new_sym();
            if (ptr.base.size() + 1 == exps.size())
                cur_block->stmt_list.push_back(make_symbol_def_load(sym, res));
            else // partial dereference
                cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(sym, res, new KoopaVal(0)));
        }

        return new KoopaVal(sym);
    }
    else
        assert(false);
}

KoopaVal *InitValAST::SynIR() const
{
    return exp->SynIR();
}

KoopaVal *LOrExpAST::SynIRshortCircuit() const
{
    if (prod_id == 1)
        return land_exp->SynIRshortCircuit();
    else
    {
        string sym = "\%lor" + to_string(lor_cnt);
        string lb_r = "\%lor_" + to_string(lor_cnt) + "_right", lb_e = "\%lor_" + to_string(lor_cnt) + "_end";
        lor_cnt++;

        // alloc
        cur_block->stmt_list.push_back(make_symbol_def_alloc(sym, new KoopaType(KP_TY_I32)));
        // store t
        cur_block->stmt_list.push_back(make_store(new KoopaVal(1), sym));
        auto lhs = lor_exp->SynIRshortCircuit();
        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaBranch(lhs, lb_e, lb_r));
        endCurBlock();

        createNewBlock(lb_r);
        auto rhs = land_exp->SynIRshortCircuit();
        // store f
        string tmp = make_new_symbol_def("ne", rhs, new KoopaVal(0));
        cur_block->stmt_list.push_back(make_store(new KoopaVal(tmp), sym));
        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaJump(lb_e));
        endCurBlock();

        createNewBlock(lb_e);
        string new_sym = get_new_sym();
        // load
        cur_block->stmt_list.push_back(make_symbol_def_load(new_sym, sym));
        return new KoopaVal(new_sym);
    }
}

KoopaVal *LAndExpAST::SynIRshortCircuit() const
{
    if (prod_id == 1)
        return eq_exp->SynIR();
    else
    {
        string sym = "\%land" + to_string(land_cnt);
        string lb_r = "\%land_" + to_string(land_cnt) + "_right", lb_e = "\%land_" + to_string(land_cnt) + "_end";
        land_cnt++;

        // alloc
        cur_block->stmt_list.push_back(make_symbol_def_alloc(sym, new KoopaType(KP_TY_I32)));
        // store t
        cur_block->stmt_list.push_back(make_store(new KoopaVal(0), sym));
        auto lhs = land_exp->SynIRshortCircuit();
        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaBranch(lhs, lb_r, lb_e));
        endCurBlock();

        createNewBlock(lb_r);
        auto rhs = eq_exp->SynIR();
        // store f
        string tmp = make_new_symbol_def("ne", rhs, new KoopaVal(0));
        cur_block->stmt_list.push_back(make_store(new KoopaVal(tmp), sym));
        cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaJump(lb_e));
        endCurBlock();

        createNewBlock(lb_e);
        string new_sym = get_new_sym();
        // load
        cur_block->stmt_list.push_back(make_symbol_def_load(new_sym, sym));
        return new KoopaVal(new_sym);
    }
}
// KoopaVal *ExpAST::shortCircuit(const string &t_entry, const string &f_entry) const
// {
//     return lor_exp->shortCircuit(t_entry, f_entry);
// }

// KoopaVal *LOrExpAST::shortCircuit(const string &t_entry, const string &f_entry) const
// {
//     if (prod_id == 1)
//         return land_exp->shortCircuit(t_entry, f_entry);
//     else
//     {
//         string sym_r = "\%lor_" + to_string(lor_cnt) + "_right";
//         auto lhs = lor_exp->shortCircuit(t_entry, sym_r);
//         lor_cnt++;

//         cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaBranch(lhs, t_entry, sym_r));
//         endCurBlock();

//         createNewBlock(sym_r);
//         return land_exp->shortCircuit(t_entry, f_entry);
//     }
// }

// KoopaVal *LAndExpAST::shortCircuit(const string &t_entry, const string &f_entry) const
// {
//     if (prod_id == 1)
//         return eq_exp->SynIR();
//     else
//     {
//         string sym_r = "\%land_" + to_string(lor_cnt) + "_right";
//         auto lhs = land_exp->shortCircuit(sym_r, f_entry);
//         land_cnt++;
//         cur_block->end_stmt = make_unique<KoopaEndStmt>(new KoopaBranch(lhs, sym_r, f_entry));
//         endCurBlock();

//         createNewBlock(sym_r);
//         return eq_exp->SynIR();
//     }
// }
// void CompUnitAST::synRawProgram(koopa_raw_program_t &p)
// {
//     // clog << "CompUnitAST syn 0" << endl;
//     FuncDefAST *func_def_p = (FuncDefAST *)this->func_def.get();

//     init_koopa_raw_slice_t(p.funcs, 1, KOOPA_RSIK_FUNCTION);
//     init_koopa_raw_slice_t(p.values, 0, KOOPA_RSIK_VALUE);
//     // clog << "CompUnitAST syn 1" << endl;
//     p.funcs.buffer[0] = func_def_p->synRawFunction();
//     // clog << "CompUnitAST syn 2" << endl;
// }

// koopa_raw_function_t FuncDefAST::synRawFunction()
// {
//     auto p = new koopa_raw_function_data_t;
//     // clog << "FuncDefAST syn 0"  << endl;
//     p->name = this->ident.c_str();

//     // clog << "FuncDefAST syn 1" << endl;
//     FuncTypeAST *func_type_p = (FuncTypeAST *)this->func_type.get();
//     // p->ty = func_type_p->synRawType();
//     auto q = new koopa_raw_type_kind_t;
//     q->tag = KOOPA_RTT_FUNCTION;
//     q->data.function.ret = func_type_p->synRawType();
//     init_koopa_raw_slice_t(q->data.function.params, 0, KOOPA_RSIK_VALUE);
//     p->ty = q;

//     // clog << "FuncDefAST syn 2" << endl;

//     init_koopa_raw_slice_t(p->bbs, 1, KOOPA_RSIK_BASIC_BLOCK);
//     BlockAST *block_p = (BlockAST *)this->block.get();
//     p->bbs.buffer[0] = block_p->synBasicBlock();

//     init_koopa_raw_slice_t(p->params, 0, KOOPA_RSIK_VALUE);

//     return p;
// }

// koopa_raw_type_t FuncTypeAST::synRawType()
// {
//     auto p = new koopa_raw_type_kind_t;
//     p->tag = KOOPA_RTT_INT32;

//     return p;
// }

// koopa_raw_basic_block_t BlockAST::synBasicBlock()
// {
//     auto p = new koopa_raw_basic_block_data_t;
//     // clog << "BLockAST syn 0" << endl;
//     p->name = "\%entry";

//     init_koopa_raw_slice_t(p->insts, 1, KOOPA_RSIK_VALUE);
//     StmtAST *stmt_p = (StmtAST *)this->stmt.get();
//     p->insts.buffer[0] = stmt_p->synValue();

//     init_koopa_raw_slice_t(p->used_by, 0, KOOPA_RSIK_VALUE);
//     return p;
// }

// koopa_raw_value_t StmtAST::synValue()
// {
//     auto p = new koopa_raw_value_data_t;
//     p->kind.tag = KOOPA_RVT_RETURN;

//     NumberAST *number_p = (NumberAST *)this->number.get();
//     p->kind.data.ret.value = number_p->synValue();

//     p->name = nullptr;

//     auto pty = new koopa_raw_type_kind_t;
//     pty->tag = KOOPA_RTT_UNIT;
//     p->ty = pty;

//     init_koopa_raw_slice_t(p->used_by, 0, KOOPA_RSIK_VALUE);

//     return p;
// }

// koopa_raw_value_t NumberAST::synValue()
// {
//     auto p = new koopa_raw_value_data_t;
//     p->kind.tag = KOOPA_RVT_INTEGER;
//     p->kind.data.integer.value = this->int_const;

//     p->name = nullptr;

//     auto pty = new koopa_raw_type_kind_t;
//     pty->tag = KOOPA_RTT_INT32;
//     p->ty = pty;

//     init_koopa_raw_slice_t(p->used_by, 0, KOOPA_RSIK_VALUE);

//     return p;
// }