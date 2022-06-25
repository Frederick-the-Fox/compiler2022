#include "arrayaux.h"
#include "astaux.h"

inline string make_sym(string s, int d)
{
    return d == 0 ? s : ("%" + s.substr(1) + "_D" + to_string(d));
}

// element pointer
// #define EPTR(x) ((x) < 0 ? arr.name : ptr[x])

int inc(const Array &arr, vector<int> &dIndex)
{
    int len = arr.dim.size();
    dIndex[len - 1]++;
    int d = len - 1;
    for (; d >= 0; --d)
    {
        if (dIndex[d] == arr.dim[d])
        {
            dIndex[d] = 0;
            if (d > 0)
                dIndex[d - 1]++;
        }
        else
            break;
    }
    return d;
}

// dIndex[D - 1] is fixed, traverse from dim D to dim n-1 (start by 0)
void init_local_const_array_rec(const Array &arr, vector<int> &dIndex, vector<string> &ptr, int D, const ConstInitValAST *init_val)
{
    assert(init_val->prod_id == 2);
    auto &list = init_val->const_init_vals;
    int len = arr.dim.size();
    int size = 1;
    for (int i = D; i < len; ++i)
        size *= arr.dim[i];
    if (D == 0)
        for (int i = 0; i < len; ++i)
        {
            dIndex[i] = 0;
            ptr[i + 1] = get_new_sym();
            cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(ptr[i + 1], ptr[i], new KoopaVal(0)));
        }
    int cnt = 0;
    for (auto iter = list.begin(); iter != list.end(); iter++)
    {
        auto c = (*iter).get();
        if (c->prod_id == 1) // is scala
        {
            ++cnt;
            cur_block->stmt_list.push_back(make_store(new KoopaVal(c->eval()), ptr[len]));

            int d = inc(arr, dIndex);
            if (d < 0)
                return;
            assert(d >= D || cnt == size);

            for (int i = d; i < len; ++i)
            {
                ptr[i + 1] = get_new_sym();
                cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(ptr[i + 1], ptr[i], new KoopaVal(dIndex[i])));
            }
        }
        else if (c->prod_id == 2) // is init list
        {
            int d = len - 1, agn = 1;
            for (; dIndex[d] == 0 && d > D; d--)
                agn = agn * arr.dim[d];
            assert(d < len - 1); // not aligned
            init_local_const_array_rec(arr, dIndex, ptr, d + 1, c);
            cnt += agn;
        }
    }

    if (cnt == size)
        return;
    for (; cnt < size; ++cnt)
    {
        cur_block->stmt_list.push_back(make_store(new KoopaVal(0), ptr[len]));
        int d = inc(arr, dIndex);
        if (d < 0)
            return;
        for (int i = d; i < len; ++i)
        {
            ptr[i + 1] = get_new_sym();
            cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(ptr[i + 1], ptr[i], new KoopaVal(dIndex[i])));
        }
    }
}

void init_local_const_array(const Array &arr, const ConstInitValAST *init_val)
{
    assert(init_val->prod_id == 2);
    int len = arr.dim.size();
    vector<int> dIndex(len);
    vector<string> ptr(len + 1);
    ptr[0] = arr.name;
    init_local_const_array_rec(arr, dIndex, ptr, 0, init_val);
}

void init_local_var_array_rec(const Array &arr, vector<int> &dIndex, vector<string> &ptr, int D, const InitValAST *init_val)
{
    auto &list = init_val->init_vals;
    int len = arr.dim.size();
    int size = 1;
    for (int i = D; i < len; ++i)
        size *= arr.dim[i];

    if (D == 0)
        for (int i = 0; i < len; ++i)
        {
            dIndex[i] = 0;
            ptr[i + 1] = get_new_sym();
            cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(ptr[i + 1], ptr[i], new KoopaVal(0)));
        }

    int cnt = 0;
    for (auto iter = list.begin(); iter != list.end(); iter++)
    {
        auto c = (*iter).get();
        if (c->prod_id == 1) // is scala
        {
            ++cnt;
            auto val = c->SynIR();
            cur_block->stmt_list.push_back(make_store(val, ptr[len]));

            int d = inc(arr, dIndex);
            if (d < 0)
                return;
            assert(d >= D || cnt == size);
            for (int i = d; i < len; ++i)
            {
                ptr[i + 1] = get_new_sym();
                cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(ptr[i + 1], ptr[i], new KoopaVal(dIndex[i])));
            }
        }
        else if (c->prod_id == 2) // is init list
        {
            int d = len - 1, agn = 1;
            for (; dIndex[d] == 0 && d > D; d--)
                agn = agn * arr.dim[d];
            assert(d < len - 1); // not aligned
            init_local_var_array_rec(arr, dIndex, ptr, d + 1, c);
            cnt += agn;
        }
    }

    if (cnt == size)
        return;
    for (; cnt < size; ++cnt)
    {
        cur_block->stmt_list.push_back(make_store(new KoopaVal(0), ptr[len]));
        int d = inc(arr, dIndex);
        if (d < 0)
            return;
        for (int i = d; i < len; ++i)
        {
            ptr[i + 1] = get_new_sym();
            cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(ptr[i + 1], ptr[i], new KoopaVal(dIndex[i])));
        }
    }
}

void init_local_var_array(const Array &arr, const InitValAST *init_val)
{
    if (init_val->prod_id == 1)
    {
        auto val = init_val->SynIR();
        cur_block->stmt_list.push_back(make_store(val, arr.name));
    }
    else
    {
        assert(init_val->prod_id == 2);
        int len = arr.dim.size();
        vector<int> dIndex(len);
        vector<string> ptr(len + 1);
        ptr[0] = arr.name;
        init_local_var_array_rec(arr, dIndex, ptr, 0, init_val);
    }
}

void init_global_const_array_rec(const Array &arr, vector<int> &dIndex, vector<KoopaAggregate *> &aggr, int D, const ConstInitValAST *init_val)
{
    auto &list = init_val->const_init_vals;
    int len = arr.dim.size();
    int size = 1;
    for (int i = D; i < len; ++i)
        size *= arr.dim[i];

    int cnt = 0;
    for (auto iter = list.begin(); iter != list.end(); iter++)
    {
        auto c = (*iter).get();
        if (c->prod_id == 1) // is scala
        {
            ++cnt;
            aggr[len - 1]->initializers.push_back(make_unique<KoopaInitializer>(c->eval()));

            int d = inc(arr, dIndex);
            if (d < 0)
                return;
            assert(d >= D || cnt == size);

            for (int i = len - 1; i > d; --i)
            {
                aggr[i - 1]->initializers.push_back(make_unique<KoopaInitializer>(aggr[i]));
                aggr[i] = new KoopaAggregate();
            }
        }
        else if (c->prod_id == 2) // is init list
        {
            int d = len - 1, agn = 1;
            for (; dIndex[d] == 0 && d > D; d--)
                agn = agn * arr.dim[d];
            assert(d < len - 1); // not aligned
            init_global_const_array_rec(arr, dIndex, aggr, d + 1, c);
            cnt += agn;
        }
    }

    if (cnt == size)
        return;
    for (; cnt < size; ++cnt)
    {
        aggr[len - 1]->initializers.push_back(make_unique<KoopaInitializer>(0));
        int d = inc(arr, dIndex);
        if (d < 0)
            return;
        for (int i = len - 1; i > d; --i)
        {
            aggr[i - 1]->initializers.push_back(make_unique<KoopaInitializer>(aggr[i]));
            aggr[i] = new KoopaAggregate();
        }
    }
}

KoopaInitializer *init_global_const_array(const Array &arr, const ConstInitValAST *init_val)
{
    assert(init_val->prod_id == 2);
    int len = arr.dim.size();
    vector<int> dIndex(len);
    vector<KoopaAggregate *> aggr(len);
    for (int i = 0; i < len; ++i)
        aggr[i] = new KoopaAggregate();
    init_global_const_array_rec(arr, dIndex, aggr, 0, init_val);
    return new KoopaInitializer(aggr[0]);
}

void init_global_var_array_rec(const Array &arr, vector<int> &dIndex, vector<KoopaAggregate *> &aggr, int D, const InitValAST *init_val)
{

    auto &list = init_val->init_vals;
    int len = arr.dim.size();
    int size = 1;
    for (int i = D; i < len; ++i)
        size *= arr.dim[i];

    int cnt = 0;
    for (auto iter = list.begin(); iter != list.end(); iter++)
    {
        auto c = (*iter).get();
        if (c->prod_id == 1) // is scala
        {
            ++cnt;
            aggr[len - 1]->initializers.push_back(make_unique<KoopaInitializer>(c->exp->eval()));

            int d = inc(arr, dIndex);
            for (int i = len - 1; i > max(d, 0); --i)
            {
                aggr[i - 1]->initializers.push_back(make_unique<KoopaInitializer>(aggr[i]));
                aggr[i] = new KoopaAggregate();
            }
            if (d < 0)
                return;
            assert(d >= D || cnt == size);
        }
        else if (c->prod_id == 2) // is init list
        {
            int d = len - 1, agn = 1;
            for (; dIndex[d] == 0 && d > D; d--)
                agn = agn * arr.dim[d];
            assert(d < len - 1); // not aligned
            init_global_var_array_rec(arr, dIndex, aggr, d + 1, c);
            cnt += agn;
        }
    }

    if (cnt == size)
        return;
    for (; cnt < size; ++cnt)
    {
        aggr[len - 1]->initializers.push_back(make_unique<KoopaInitializer>(0));
        int d = inc(arr, dIndex);
        for (int i = len - 1; i > max(0, d); --i)
        {
            aggr[i - 1]->initializers.push_back(make_unique<KoopaInitializer>(aggr[i]));
            aggr[i] = new KoopaAggregate();
        }
        if (d < 0)
            return;
    }
}

KoopaInitializer *init_global_var_array(const Array &arr, const InitValAST *init_val)
{
    if (init_val->prod_id == 1)
        return new KoopaInitializer(init_val->exp->eval());
    else
    {
        assert(init_val->prod_id == 2);
        int len = arr.dim.size();
        vector<int> dIndex(len);
        vector<KoopaAggregate *> aggr(len);
        for (int i = 0; i < len; ++i)
            aggr[i] = new KoopaAggregate();
        init_global_var_array_rec(arr, dIndex, aggr, 0, init_val);
        return new KoopaInitializer(aggr[0]);
    }
}

KoopaType *get_array_type(const vector<int> &dim)
{
    auto ty = new KoopaType(KP_TY_I32);
    for (int i = dim.size() - 1; i >= 0; --i)
    {
        auto tmp = new KoopaType(new KoopaArrayType(ty, dim[i]));
        ty = tmp;
    }
    return ty;
}

KoopaType *get_pointer_type(const vector<int> &base)
{
    return new KoopaType(new KoopaPointerType(get_array_type(base)));
}

string array_access(const Array &arr, const vector<unique_ptr<ExpAST>> &exps)
{
    int len = exps.size();
    // assert(exps.size() == len);
    string prev = arr.name;
    for (int i = 0; i < len; ++i)
    {
        auto val = exps[i]->SynIR();
        string ptr = get_new_sym();
        cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(ptr, prev, val));
        prev = ptr;
    }
    return prev;
}

string pointer_access(string str, const vector<int> &base, const vector<unique_ptr<ExpAST>> &exps)
{
    string sym2 = get_new_sym();
    auto val = exps[0]->SynIR();
    cur_block->stmt_list.push_back(make_symbol_def_get_pointer(sym2, str, val));

    int len = exps.size();
    if (len == 1)
        return sym2;

    // string sym3 = get_new_sym();
    // cur_block->stmt_list.push_back(make_symbol_def_load(sym3, sym2));

    string prev = sym2;
    for (int i = 1; i < len; ++i) // start from 1
    {
        auto val = exps[i]->SynIR();
        string ptr = get_new_sym();
        cur_block->stmt_list.push_back(make_symbol_def_get_element_pointer(ptr, prev, val));
        prev = ptr;
    }
    return prev;
}