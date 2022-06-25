#include "symboltable.h"

using namespace std;

unordered_map<string, FunInfo> FTable;

void initFTable()
{
    assert(FTable.empty());
    FTable.insert({"getint", FunInfo("int")});
    FTable.insert({"getch", FunInfo("int")});
    FTable.insert({"getarray", FunInfo("int")});
    FTable.insert({"putint", FunInfo("void")});
    FTable.insert({"putch", FunInfo("void")});
    FTable.insert({"putarray", FunInfo("void")});
    FTable.insert({"starttime", FunInfo("void")});
    FTable.insert({"stoptime", FunInfo("void")});
}

class SymbolTable
{
public:
    unordered_map<string, UType> M;
    SymbolTable *fatherST;
    static int ST_cnt;
    // void insert_const(string sym, int const_val);
    // void insert_var(string sym, string name);
    // bool exist(string sym);
    // bool is_const(string sym);
    // bool is_var(string sym);
    // int query(string sym);
};

int SymbolTable::ST_cnt = -1;

SymbolTable *CurST = nullptr;
SymbolTable *GlobalST = nullptr;

// void SymbolTable::insert_const(string sym, int const_val)
// {
//     assert(!exist(sym));
//     M.insert({sym, Constant(const_val)});
// }

// void SymbolTable::insert_var(string sym, string name)
// {
//     assert(!exist(sym));
//     M.insert({sym, Variable()});
// }
// bool SymbolTable::exist(string sym)
// {
//     return (M.count(sym) > 0);
// }
// bool SymbolTable::is_const(string sym)
// {
//     assert(exist(sym));
//     return M[sym].index() == 0;
// }
// bool SymbolTable::is_var(string sym)
// {
//     assert(exist(sym));
//     return M[sym].index() == 1;
// }

// int SymbolTable::query(string sym)
// {
//     // clog << "query: " << sym << endl;
//     assert(is_const(sym));
//     int ret = (get<Constant>(M[sym])).val;
//     // clog << "query success: (" << sym << ", " << ret << ")\n";
//     return ret;
// }

void initST()
{
    assert(GlobalST == nullptr);
    buildNewST();
    GlobalST = CurST;
}

void buildNewST()
{
    SymbolTable *new_st = new SymbolTable;
    new_st->fatherST = CurST;
    CurST = new_st;
    SymbolTable::ST_cnt++;
}

void quitCurST()
{
    SymbolTable *del_st = CurST;
    CurST = del_st->fatherST;
    if (del_st == GlobalST)
        GlobalST = nullptr;
    delete del_st;
}

bool STExist(string sym)
{
    for (auto p = CurST; p != nullptr; p = p->fatherST)
        if (p->M.count(sym) != 0)
            return 1;
    return 0;
}

// unique_ptr<KoopaVal> STQuery(string sym)
// {
//     for (auto p = CurST; p != nullptr; p = p->fatherST)
//         if (p->M.count(sym) != 0)
//         {
//             auto tmp = p->M[sym];
//             if (tmp.index() == 0)
//                 return make_unique<KoopaVal>(get<Constant>(tmp).val);
//             else if (tmp.index() == 1)
//                 return make_unique<KoopaVal>(get<Array>(tmp).name);
//             else
//             {
//                 // clog << "fail: " << sym << endl;
//                 assert(false);
//             }
//         }
//     assert(false);
//     return nullptr;
// }

void STInsertConst(string sym, int const_val)
{
    // assert(CurST->M.count(sym) == 0);
    // CurST->M.insert({sym, Constant(const_val)});
    CurST->M[sym] = Constant(const_val);
}

// var is 0-dimension array
void STInsertVar(string sym, string name)
{
    CurST->M[sym] = Array(name);
}

void STInsertArray(string sym, const Array &array)
{
    // clog << "insert array:" << sym << " ST:" << STGetCnt() << endl;
    CurST->M[sym] = array;
}

void STInsertPointer(string sym, const Pointer &pointer)
{
    // clog << "insert pointer:" << sym << " ST:" << STGetCnt() << endl;
    CurST->M[sym] = pointer;
}

int STGetCnt()
{
    return SymbolTable::ST_cnt;
}

// bool isGlobalVar(string sym)
// {
//     return GlobalST->M.count(sym) == 1 && GlobalST->M[sym].index() == 1;
// }
// void STInsertGlobalVar(string sym, string name, int init_val)
// {
//     assert(CurST == GlobalST);
//     CurST->M[sym] = Variable(name, init_val);
// }
// int STGetGlobalVarInitVal(string sym)
// {
//     return get<Variable>(GlobalST->M[sym]).init_val;
// }

UType STGet(string sym)
{
    for (auto p = CurST; p != nullptr; p = p->fatherST)
        if (p->M.count(sym) != 0)
            return p->M[sym];
    assert(false);
}

int STGetConst(string sym)
{
    auto tmp = STGet(sym);
    assert(tmp.index() == 0);
    return get<Constant>(tmp).val;
}

Array STGetArray(string sym)
{
    auto tmp = STGet(sym);
    assert(tmp.index() == 1);
    return get<Array>(tmp);
}

Pointer STGetPointer(string sym)
{
    auto tmp = STGet(sym);
    assert(tmp.index() == 2);
    return get<Pointer>(tmp);
}

// Pointer STGetPointer(string sym)
// {
//     for (auto p = CurST; p != nullptr; p = p->fatherST)
//         if (p->M.count(sym) != 0)
//         {
//             auto tmp = p->M[sym];
//             assert(tmp.index() == 3);
//             return get<Array>(tmp);
//         }
// }