#ifndef __GENCODE__
#define __GENCODE__

#include "koopa.h"
#include "assert.h"
#include <string>
#include <cstring>
#include <iostream>
#include <unordered_map>

using namespace std;

extern const string regname[32];

typedef enum
{
    ZERO,
    RA,
    SP,
    GP,
    TP,
    T0,
    T1,
    T2,
    S0,
    S1,
    A0,
    A1,
    A2,
    A3,
    A4,
    A5,
    A6,
    A7,
    S2,
    S3,
    S4,
    S5,
    S6,
    S7,
    S8,
    S9,
    S10,
    S11,
    T3,
    T4,
    T5,
    T6,
} RegNo;

RegNo allocReg(koopa_raw_value_t p, bool &imm, string &s);
RegNo findFreeReg();

string make_binary_code(string op, RegNo rd, RegNo rs1, RegNo rs2);
string gen_lw(RegNo rd, int offset, RegNo rs);
string gen_sw(RegNo rs2, int offset, RegNo rs1);
string make_li(RegNo rd, int imm);
string make_bnez(RegNo rs, string label);
string make_j(string label);

void genCode(const koopa_raw_program_t &t, string &s);
void genCode(const koopa_raw_slice_t &slice, string &s);
void genCode(const koopa_raw_function_t &func, string &s);
void genCode(const koopa_raw_basic_block_t &bb, string &s);
void genCode(const koopa_raw_value_t &value, string &s);
void genCode(const koopa_raw_return_t &ret, string &s);
// string genCode(const koopa_raw_integer_t &inte);
RegNo genCode(const koopa_raw_binary_t &binary, string &s);
void genCode(const koopa_raw_store_t &store, string &s);
RegNo genCode(const koopa_raw_load_t &load, string &s);
void genCode(const koopa_raw_branch_t &branch, string &s);
void genCode(const koopa_raw_jump_t &jump, string &s);
void genCode(const koopa_raw_call_t &call, string &s);
void genCodeInit(const koopa_raw_value_t &init, size_t sz, string &s);
RegNo genCode(const koopa_raw_get_elem_ptr_t &get_elem_ptr, string &s);
void genCode(const koopa_raw_aggregate_t &aggregate, size_t sz, string &s);
RegNo genCode(const koopa_raw_get_ptr_t &get_elem_ptr, string &s);
#endif