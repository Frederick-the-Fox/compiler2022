#ifndef __SYMBOLTABLE__
#define __SYMBOLTABLE__

#include <string>
#include <unordered_map>
#include <variant>
#include <iostream>
#include "assert.h"
#include "irdefinition.h"
using namespace std;

class Constant
{
public:
    int val;
    Constant() {}
    Constant(int _val) { val = _val; }
};
// class Variable
// {
// public:
//     string name;
//     int init_val;
//     Variable() {}
//     Variable(string _name) { name = _name; }
//     Variable(string _name, int _init_val) : name(_name), init_val(_init_val) {}
// };

class Array
{
public:
    string name;
    vector<int> dim;
    Array() {}
    Array(string _name) : name(_name) {}
};

class Pointer
{
public:
    string name;
    vector<int> base;
    Pointer() {}
    Pointer(string _name, const vector<int> &_base) : name(_name), base(_base) {}
};

typedef variant<Constant, Array, Pointer> UType;

void initST();
void buildNewST();
void quitCurST();
int STGetCnt();
bool STExist(string sym);
// unique_ptr<KoopaVal> STQuery(string sym);
void STInsertConst(string sym, int const_val);
void STInsertVar(string sym, string name);
void STInsertPointer(string sym, const Pointer &pointer);

// bool isGlobalVar(string sym);
// void STInsertGlobalVar(string sym, string name, int init_val);
// int STGetGlobalVarInitVal(string sym);
void STInsertArray(string sym, const Array &array);

UType STGet(string sym);
int STGetConst(string sym);
Array STGetArray(string sym);
Pointer STGetPointer(string sym);

class FunInfo
{
public:
    string type;
    FunInfo() {}
    FunInfo(string _type) : type(_type) {}
};

void initFTable();

extern unordered_map<string, FunInfo> FTable;

#endif