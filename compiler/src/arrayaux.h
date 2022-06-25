#ifndef __ARRAYAUX__
#define __ARRAYAUX__

#include "astdefinition.h"

void init_local_const_array(const Array &arr, const ConstInitValAST *init_val);
void init_local_var_array(const Array &arr, const InitValAST *init_val);
KoopaInitializer *init_global_const_array(const Array &arr, const ConstInitValAST *init_val);
KoopaInitializer *init_global_var_array(const Array &arr, const InitValAST *init_val);

KoopaType *get_array_type(const vector<int> &dim);
KoopaType *get_pointer_type(const vector<int> &base);
string array_access(const Array &arr, const vector<unique_ptr<ExpAST>> &exps);
string pointer_access(string str, const vector<int> &base, const vector<unique_ptr<ExpAST>> &exps);

#endif