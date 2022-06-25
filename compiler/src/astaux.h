#ifndef __ASTAUX__
#define __ASTAUX__

#include "astdefinition.h"

unique_ptr<KoopaStmt> make_symbol_def_alloc(string sym, KoopaType *ty);
unique_ptr<KoopaStmt> make_symbol_def_load(string sym, string name);
KoopaStmt *make_symbol_def_fun_call(string sym, string fun_name, const vector<unique_ptr<ExpAST>> &params);
unique_ptr<KoopaStmt> make_symbol_def_get_element_pointer(string sym, string arr_name, KoopaVal *val);
unique_ptr<KoopaStmt> make_symbol_def_get_pointer(string sym, string arr_name, KoopaVal *val);
unique_ptr<KoopaStmt> make_store(KoopaVal *val, string sym);
string get_new_sym();

#endif