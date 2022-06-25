#include "astaux.h"

int sym_cnt = 0;

string get_new_sym()
{
    sym_cnt++;
    return "%" + to_string(sym_cnt);
}

unique_ptr<KoopaStmt> make_symbol_def_alloc(string sym, KoopaType *ty)
{
    // auto ty = new KoopaType(KP_TY_I32);
    auto memory_declaration = new KoopaMemoryDeclaration(ty);
    auto symbol_def = new KoopaSymbolDef(sym, memory_declaration);
    auto st = make_unique<KoopaStmt>(symbol_def);
    return st;
}

unique_ptr<KoopaStmt> make_symbol_def_load(string sym, string name)
{
    auto load = new KoopaLoad(name);
    auto symbol_def = new KoopaSymbolDef(sym, load);
    auto st = make_unique<KoopaStmt>(symbol_def);
    return st;
}

KoopaStmt *make_symbol_def_fun_call(string sym, string fun_name, const vector<unique_ptr<ExpAST>> &params)
{
    auto fun_call = new KoopaFunCall(fun_name);
    for (auto iter = params.begin(); iter != params.end(); iter++)
        fun_call->value_list.push_back(unique_ptr<KoopaVal>((*iter)->SynIR()));
    auto symbol_def = new KoopaSymbolDef(sym, fun_call);
    auto st = new KoopaStmt(symbol_def);
    return st;
}

unique_ptr<KoopaStmt> make_symbol_def_get_element_pointer(string sym, string arr_name, KoopaVal *val)
{
    auto get_element_pointer = new KoopaGetElementPointer(arr_name, val);
    auto symbol_def = new KoopaSymbolDef(sym, get_element_pointer);
    auto st = make_unique<KoopaStmt>(symbol_def);
    return st;
}

unique_ptr<KoopaStmt> make_symbol_def_get_pointer(string sym, string arr_name, KoopaVal *val)
{
    auto get_pointer = new KoopaGetPointer(arr_name, val);
    auto symbol_def = new KoopaSymbolDef(sym, get_pointer);
    auto st = make_unique<KoopaStmt>(symbol_def);
    return st;
}

unique_ptr<KoopaStmt> make_store(KoopaVal *val, string sym)
{
    auto store = new KoopaStore(val, sym);
    auto st = make_unique<KoopaStmt>(store);
    return st;
}