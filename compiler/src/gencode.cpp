#include "gencode.h"
#include "koopaaux.h"

// 函数声明略
// ...

// 访问 raw program

using namespace std;

const string regname[] = {"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

// #define ZERO 0
// #define RA 1
// #define SP 2
// #define GP 3
// #define TP 4
// #define T0 5
// #define T1 6
// #define T2 7
// #define S0 8
// #define S1 9
// #define A0 10
// #define A1 11
// #define A2 12
// #define A3 13
// #define A4 14
// #define A5 15
// #define A6 16
// #define A7 17
// #define S2 18
// #define S3 19
// #define S4 20
// #define S5 21
// #define S6 22
// #define S7 23
// #define S8 24
// #define S9 25
// #define S10 26
// #define S11 27
// #define T3 28
// #define T4 29
// #define T5 30
// #define T6 31

#define CASTLABEL(str) (string((str) + 1))
#define UPTO16(x) (((x + 15) >> 4) << 4)

koopa_raw_function_t cur_raw_fun = nullptr;

int cur_len_frame = -1;
int cur_has_call = -1;

string *cur_s;

bool usedReg[32];

bool operator==(const koopa_raw_binary_t &a, const koopa_raw_binary_t &b)
{
    return a.lhs == b.lhs && a.op == b.op && a.rhs == b.rhs;
}
struct hash_koopa_raw_binary_t
{
    size_t operator()(const koopa_raw_binary_t &a) const
    {
        return reinterpret_cast<std::uintptr_t>(a.lhs) + 233 * reinterpret_cast<std::uintptr_t>(a.lhs) + a.op * 66667;
    }
};

unordered_map<koopa_raw_binary_t, int, hash_koopa_raw_binary_t> MReg;

unordered_map<koopa_raw_value_t, int> LocalTmpRelAddr;
unordered_map<koopa_raw_value_t, int> LocalAllocRelAddr;

string make_binary_code(string op, RegNo rd, RegNo rs1, RegNo rs2)
{
    return "    " + op + " " + regname[rd] + ", " + regname[rs1] + ", " + regname[rs2] + "\n";
}

string make_addi(RegNo rd, RegNo rs1, int imm12)
{
    assert(imm12 < 2048 && imm12 >= -2048);
    return "    addi " + regname[rd] + ", " + regname[rs1] + ", " + to_string(imm12) + "\n";
}

// string make_add(RegNo rd, RegNo rs1, RegNo rs2)
// {
//     return "    add " + regname[rd] + ", " + regname[rs1] + ", " + regname[rs2] + "\n";
// }

string gen_addi(RegNo rd, RegNo rs1, int imm)
{
    string s;
    if (imm >= 2048 || imm < -2048)
    {
        RegNo reg = findFreeReg();
        s += make_li(reg, imm);
        s += make_binary_code("add", rd, rs1, reg);
        usedReg[reg] = 0;
    }
    else
        s += make_addi(rd, rs1, imm);
    return s;
}

string gen_lw(RegNo rd, int offset, RegNo rs)
{
    string s = "";
    if (offset < 2048 && offset >= -2048)
        s = "    lw " + regname[rd] + ", " + to_string(offset) + "(" + regname[rs] + ")\n";
    else
    {
        RegNo tmpreg = findFreeReg();
        s += make_li(tmpreg, offset);
        s += make_binary_code("add", tmpreg, tmpreg, rs);
        s += "    lw " + regname[rd] + ", 0(" + regname[tmpreg] + ")\n";
        usedReg[tmpreg] = 0;
    }
    return s;
}
string gen_sw(RegNo rs2, int offset, RegNo rs1)
{
    string s = "";
    if (offset < 2048 && offset >= -2048)
        s = "    sw " + regname[rs2] + ", " + to_string(offset) + "(" + regname[rs1] + ")\n";
    else
    {
        RegNo tmpreg = findFreeReg();
        s += make_li(tmpreg, offset);
        s += make_binary_code("add", tmpreg, tmpreg, rs1);
        s += "    sw " + regname[rs2] + ", 0(" + regname[tmpreg] + ")\n";
        usedReg[tmpreg] = 0;
    }
    return s;
}

string make_li(RegNo rd, int imm)
{
    return "    li " + regname[rd] + ", " + to_string(imm) + "\n";
}

string make_bnez(RegNo rs, string label)
{
    return "    bnez " + regname[rs] + ", " + label + "\n";
}

string make_j(string label)
{
    return "    j " + label + "\n";
}

string make_call(string label)
{
    return "    call " + label + "\n";
}

string make_la(RegNo rd, string label)
{
    return "    la " + regname[rd] + ", " + label + "\n";
}

void genCode(const koopa_raw_program_t &program, string &s)
{
    // 执行一些其他的必要操作
    // ...
    // 访问所有全局变量
    cur_s = &s;
    genCode(program.values, s);
    // 访问所有函数
    genCode(program.funcs, s);
}

// 访问 raw slice
void genCode(const koopa_raw_slice_t &slice, string &s)
{
    for (size_t i = 0; i < slice.len; ++i)
    {
        auto ptr = slice.buffer[i];
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            // 访问函数
            genCode(reinterpret_cast<koopa_raw_function_t>(ptr), s);
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            // 访问基本块
            genCode(reinterpret_cast<koopa_raw_basic_block_t>(ptr), s);
            break;
        case KOOPA_RSIK_VALUE:
            // 访问指令
            genCode(reinterpret_cast<koopa_raw_value_t>(ptr), s);
            break;
        default:
            // 我们暂时不会遇到其他内容, 于是不对其做任何处理
            assert(false);
        }
    }
}

// 访问函数
void genCode(const koopa_raw_function_t &func, string &s)
{
    if (func->bbs.len == 0)
        return;
    LocalTmpRelAddr.clear();
    LocalAllocRelAddr.clear();
    assert(cur_raw_fun == nullptr);
    cur_raw_fun = func;
    assert(cur_len_frame == -1);
    cur_len_frame = 0;
    assert(cur_has_call == -1);
    cur_has_call = 0;
    // assert(func->params.len <= 8);
    int p8 = 0;

    s += "\n    .text\n";
    s += "    .globl " + string(func->name + 1) + "\n" + string(func->name + 1) + ":\n";

    assert(func->bbs.kind == KOOPA_RSIK_BASIC_BLOCK);
    for (size_t k = 0; k < func->bbs.len; ++k)
    {
        auto bb = reinterpret_cast<koopa_raw_basic_block_t>(func->bbs.buffer[k]);
        assert(bb->insts.kind == KOOPA_RSIK_VALUE);
        for (size_t i = 0; i < bb->insts.len; ++i)
        {
            auto ptr = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[i]);
            if (ptr->kind.tag == KOOPA_RVT_ALLOC)
            {
                LocalAllocRelAddr[ptr] = cur_len_frame;
                cur_len_frame += get_pointer_size(ptr->ty);
            }
            else if (ptr->ty->tag != KOOPA_RTT_UNIT)
            {
                LocalTmpRelAddr[ptr] = cur_len_frame;
                cur_len_frame += 4;
            }

            if (ptr->kind.tag == KOOPA_RVT_CALL)
            {
                cur_has_call = 1;
                p8 = max(p8, (int)ptr->kind.data.call.args.len);
            }
        }
    }

    p8 = max(p8 - 8, 0) * 4;
    for (auto iter = LocalTmpRelAddr.begin(); iter != LocalTmpRelAddr.end(); iter++)
        iter->second += p8;
    for (auto iter = LocalAllocRelAddr.begin(); iter != LocalAllocRelAddr.end(); iter++)
        iter->second += p8;
    cur_len_frame += p8;

    if (cur_has_call)
        cur_len_frame += 4;

    cur_len_frame = UPTO16(cur_len_frame);

    // if (cur_len_frame >= 2048)
    // {
    //     s += make_li(T0, -cur_len_frame);
    //     s += make_binary_code("add", SP, SP, T0);
    // }
    // else
    //     s += "    addi sp, sp, " + to_string(-cur_len_frame) + "\n";

    s += gen_addi(SP, SP, -cur_len_frame);

    if (cur_has_call)
        s += gen_sw(RA, cur_len_frame - 4, SP);

    genCode(func->bbs, s);

    cur_len_frame = -1;
    cur_has_call = -1;
    cur_raw_fun = nullptr;
}

// 访问基本块+
void genCode(const koopa_raw_basic_block_t &bb, string &s)
{
    // 执行一些其他的必要操作
    // ...
    // 访问所有指令
    s += CASTLABEL(cur_raw_fun->name) + string("_") + CASTLABEL(bb->name) + ":\n";
    genCode(bb->insts, s);
}

// 访问指令
void genCode(const koopa_raw_value_t &value, string &s)
{
    // 根据指令类型判断后续需要如何访问
    assert(value != nullptr);
    // clog << "genCode value:" << value->kind.tag << endl;
    // clog << s << endl;
    const auto &kind = value->kind;
    RegNo resreg;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        // 访问 return 指令
        genCode(kind.data.ret, s);
        break;
    // case KOOPA_RVT_INTEGER:
    //     // 访问 integer 指令
    //     s += genCode(kind.data.integer);
    //     break;
    case KOOPA_RVT_BINARY:
        resreg = genCode(kind.data.binary, s);
        // if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
        // {
        //     tmpreg = findFreeReg();
        //     s += make_la(tmpreg, value->name);
        //     s += gen_sw(resreg, 0, tmpreg);
        //     usedReg[tmpreg] = 0;
        // }
        // else
        s += gen_sw(resreg, LocalTmpRelAddr[value], SP);
        usedReg[resreg] = 0;
        break;
    case KOOPA_RVT_STORE:
        genCode(kind.data.store, s);
        break;
    case KOOPA_RVT_LOAD:
        resreg = genCode(kind.data.load, s);
        // if (value->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
        // {
        //     tmpreg = findFreeReg();
        //     s += make_la(tmpreg, value->name);
        //     s += gen_sw(resreg, 0, tmpreg);
        //     usedReg[tmpreg] = 0;
        // }
        // else
        s += gen_sw(resreg, LocalTmpRelAddr[value], SP);
        usedReg[resreg] = 0;
        break;
    case KOOPA_RVT_ALLOC:
        s += "# alloc\n";
        break;
    case KOOPA_RVT_BRANCH:
        genCode(kind.data.branch, s);
        break;
    case KOOPA_RVT_JUMP:
        genCode(kind.data.jump, s);
        break;
    case KOOPA_RVT_CALL:
        genCode(kind.data.call, s);
        if (value->ty->tag != KOOPA_RTT_UNIT)
            s += gen_sw(A0, LocalTmpRelAddr[value], SP);
        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        assert(value->name != nullptr);
        s += "    .data\n    .global " + CASTLABEL(value->name) + "\n" + CASTLABEL(value->name) + ":\n";
        genCodeInit(kind.data.global_alloc.init, get_pointer_size(value->ty), s);
        break;

    case KOOPA_RVT_GET_ELEM_PTR:
        s += "# getelemptr\n";
        resreg = genCode(kind.data.get_elem_ptr, s);
        s += gen_sw(resreg, LocalTmpRelAddr[value], SP);
        usedReg[resreg] = 0;
        break;
    case KOOPA_RVT_GET_PTR:
        s += "# getptr\n";
        resreg = genCode(kind.data.get_ptr, s);
        s += gen_sw(resreg, LocalTmpRelAddr[value], SP);
        usedReg[resreg] = 0;
        break;
    default:
        // 其他类型暂时遇不到
        cerr << "ERROR KOOPA_RVT:" << kind.tag << endl;
        assert(false);
    }
    // clog << "genCode value:" << value->kind.tag << " finish." << endl;
}

// 访问对应类型指令的函数定义略
// 视需求自行实现
// ...

void genCode(const koopa_raw_return_t &ret, string &s)
{
    if (ret.value != nullptr)
    {
        if (ret.value->kind.tag == KOOPA_RVT_INTEGER)
            s += make_li(A0, ret.value->kind.data.integer.value);
        else if (ret.value->ty->tag != KOOPA_RTT_UNIT)
            s += gen_lw(A0, LocalTmpRelAddr[ret.value], SP);

        else
            assert(false);
    }

    if (cur_has_call)
    {
        s += gen_lw(RA, cur_len_frame - 4, SP);
    }
    // if (cur_len_frame >= 2048)
    // {
    //     s += make_li(T0, cur_len_frame);
    //     s += make_binary_code("add", SP, SP, T0);
    // }
    // else
    //     s += "    addi sp, sp, " + to_string(cur_len_frame) + "\n";

    s += gen_addi(SP, SP, cur_len_frame);
    s += "    ret\n";
    // s += "    ret\n";
}

// string genCode(const koopa_raw_integer_t &inte)
// {
//     string s = "";
//     s += "li a0, " + to_string(inte.value) + "\n";
//     return s;
// }

RegNo findFreeReg()
{
    int ret = -1;
    for (int i = 5; i <= 7 && ret == -1; ++i)
        if (!usedReg[i])
            ret = i;
    for (int i = 10; i <= 17 && ret == -1; ++i)
        if (!usedReg[i])
            ret = i;
    for (int i = 28; i <= 31 && ret == -1; ++i)
        if (!usedReg[i])
            ret = i;
    if (ret >= 0)
    {
        usedReg[ret] = 1;
        return (RegNo)ret;
    }
    else
    {
        clog << (*cur_s) << endl;
        assert(false);
    }
}

RegNo allocReg(koopa_raw_value_t p, string &s)
{
    RegNo reg;
    if (p->kind.tag == KOOPA_RVT_INTEGER)
    {
        if (p->kind.data.integer.value == 0)
            return ZERO; // zero
        else
        {
            reg = findFreeReg();
            s += make_li(reg, p->kind.data.integer.value);
            return reg;
        }
    }

    else if (p->ty->tag != KOOPA_RTT_UNIT)
    {
        // clog << "fail:" << p->name << endl;
        assert(LocalTmpRelAddr.count(p) != 0);
        int addr = LocalTmpRelAddr[p];
        reg = findFreeReg();
        s += gen_lw(reg, addr, SP);
        return reg;
    }
    else
        assert(false);
}

RegNo genCode(const koopa_raw_binary_t &binary, string &s)
{
    RegNo lreg, rreg, freg;
    lreg = allocReg(binary.lhs, s);
    rreg = allocReg(binary.rhs, s);
    usedReg[lreg] = 0;
    usedReg[rreg] = 0;
    freg = findFreeReg();
    // freg = 7;

    // usedReg[freg] = 0;

    switch (binary.op)
    {
    case KOOPA_RBO_EQ:
        /* code */
        s += make_binary_code("xor", freg, lreg, rreg);
        s += "    seqz " + regname[freg] + ", " + regname[freg] + "\n";
        break;
    case KOOPA_RBO_SUB:
        s += make_binary_code("sub", freg, lreg, rreg);
        break;
    case KOOPA_RBO_ADD:

        s += make_binary_code("add", freg, lreg, rreg);
        break;
    case KOOPA_RBO_MUL:
        s += make_binary_code("mul", freg, lreg, rreg);
        break;
    case KOOPA_RBO_DIV:
        s += make_binary_code("div", freg, lreg, rreg);
        break;
    case KOOPA_RBO_MOD:
        s += make_binary_code("rem", freg, lreg, rreg);
        break;
    case KOOPA_RBO_LT:
        s += make_binary_code("slt", freg, lreg, rreg);
        break;
    case KOOPA_RBO_GT:
        s += make_binary_code("sgt", freg, lreg, rreg);
        break;
    case KOOPA_RBO_LE:
        s += make_binary_code("sgt", freg, lreg, rreg);
        s += "    seqz " + regname[freg] + ", " + regname[freg] + "\n";
        break;
    case KOOPA_RBO_GE:
        s += make_binary_code("slt", freg, lreg, rreg);
        s += "    seqz " + regname[freg] + ", " + regname[freg] + "\n";
        break;
    case KOOPA_RBO_NOT_EQ:
        s += make_binary_code("xor", freg, lreg, rreg);
        s += "    snez " + regname[freg] + ", " + regname[freg] + "\n";
        break;
    case KOOPA_RBO_AND:
        s += make_binary_code("and", freg, lreg, rreg);
        break;
    case KOOPA_RBO_OR:
        s += make_binary_code("or", freg, lreg, rreg);
        break;
    default:
        break;
    }

    // s += "    sw " + regname[freg] + ", " + to_string(rel_addr) + "(sp)\n";
    return freg;
}

void genCode(const koopa_raw_store_t &store, string &s)
{
    s += "# store\n";

    RegNo reg;
    bool is_param = 0; // whether it is func param

    assert(cur_raw_fun->params.kind == KOOPA_RSIK_VALUE);
    for (size_t i = 0; i < cur_raw_fun->params.len && !is_param; ++i)
    {
        auto ptr = reinterpret_cast<koopa_raw_value_t>(cur_raw_fun->params.buffer[i]);
        assert(ptr->kind.tag == KOOPA_RVT_FUNC_ARG_REF);
        if (store.value->name != nullptr && strcmp(ptr->name, store.value->name) == 0)
        {
            is_param = 1;
            if (i < 8)
            {
                reg = RegNo(A0 + i);
                usedReg[reg] = 1;
            }
            else
            {
                reg = findFreeReg();
                s += gen_lw(reg, (i - 8) * 4 + cur_len_frame, SP);
            }
        }
    }

    if (!is_param)
        reg = allocReg(store.value, s);

    if (store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) // is global alloc
    {
        RegNo tmpreg = findFreeReg();
        s += make_la(tmpreg, CASTLABEL(store.dest->name));
        s += gen_sw(reg, 0, tmpreg);
        usedReg[tmpreg] = 0;
    }
    else if (LocalAllocRelAddr.count(store.dest) != 0) // is local alloc
    {
        s += gen_sw(reg, LocalAllocRelAddr[store.dest], SP);
    }
    else // is local tmp
    {
        RegNo regaddr = allocReg(store.dest, s);
        s += gen_sw(reg, 0, regaddr);
        usedReg[regaddr] = 0;
    }
    usedReg[reg] = 0;
}
RegNo genCode(const koopa_raw_load_t &load, string &s)
{
    s += "# load\n";
    RegNo reg = findFreeReg();
    if (LocalAllocRelAddr.count(load.src) != 0) // is local alloc
        s += gen_lw(reg, LocalAllocRelAddr[load.src], SP);
    else if (load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        s += make_la(reg, CASTLABEL(load.src->name));
        s += gen_lw(reg, 0, reg);
    }
    else // is local tmp
    {
        assert(LocalTmpRelAddr.count(load.src) != 0);
        RegNo regaddr = allocReg(load.src, s);
        s += gen_lw(reg, 0, regaddr);
        usedReg[regaddr] = 0;
    }
    return reg;
}

void genCode(const koopa_raw_branch_t &branch, string &s)
{
    RegNo reg = allocReg(branch.cond, s);
    s += make_bnez(reg, CASTLABEL(cur_raw_fun->name) + string("_") + CASTLABEL(branch.true_bb->name));
    usedReg[reg] = 0;
    s += make_j(CASTLABEL(cur_raw_fun->name) + string("_") + CASTLABEL(branch.false_bb->name));
}
void genCode(const koopa_raw_jump_t &jump, string &s)
{
    s += make_j(CASTLABEL(cur_raw_fun->name) + string("_") + CASTLABEL(jump.target->name));
}

// RegNo getParam()

void genCode(const koopa_raw_call_t &call, string &s)
{
    s += "# call\n";
    assert(call.args.kind == KOOPA_RSIK_VALUE);
    for (size_t i = 0; i < call.args.len; ++i)
    {
        auto val = reinterpret_cast<koopa_raw_value_t>(call.args.buffer[i]);
        if (i < 8)
        {
            if (val->kind.tag == KOOPA_RVT_INTEGER)
            {
                s += make_li(RegNo(A0 + i), val->kind.data.integer.value);
                usedReg[RegNo(A0 + i)] = 1;
            }
            else if (val->ty->tag != KOOPA_RTT_UNIT)
            {
                s += gen_lw(RegNo(A0 + i), LocalTmpRelAddr[val], SP);
                usedReg[RegNo(A0 + i)] = 1;
            }
        }
        else
        {
            RegNo reg = allocReg(val, s);
            s += gen_sw(reg, (i - 8) * 4, SP);
            usedReg[reg] = 0;
        }
    }

    for (size_t i = 0; i < min(8u, call.args.len); ++i)
        usedReg[RegNo(A0 + i)] = 0;

    s += make_call(CASTLABEL(call.callee->name));
}

void genCodeInit(const koopa_raw_value_t &init, size_t sz, string &s)
{
    // auto init = global_alloc.init;

    if (init->kind.tag == KOOPA_RVT_ZERO_INIT)
    {
        s += "    .zero " + to_string(sz) + "\n";
    }
    else if (init->kind.tag == KOOPA_RVT_INTEGER)
    {
        s += "    .word " + to_string(init->kind.data.integer.value) + "\n";
    }
    else if (init->kind.tag == KOOPA_RVT_AGGREGATE)
    {
        genCode(init->kind.data.aggregate, sz, s);
    }
    else
        assert(false);
}
void genCode(const koopa_raw_aggregate_t &aggregate, size_t sz, string &s)
{
    auto slice = aggregate.elems;
    sz /= slice.len;
    for (size_t i = 0; i < slice.len; ++i)
    {
        auto ptr = slice.buffer[i];
        assert(slice.kind == KOOPA_RSIK_VALUE);
        genCodeInit(reinterpret_cast<koopa_raw_value_t>(ptr), sz, s);
    }
}

RegNo genCode(const koopa_raw_get_elem_ptr_t &get_elem_ptr, string &s)
{
    RegNo reg;
    auto src = get_elem_ptr.src;
    if (LocalAllocRelAddr.count(src) != 0) // is local alloc
    {
        reg = findFreeReg();
        s += gen_addi(reg, SP, LocalAllocRelAddr[src]);
    }
    else if (LocalTmpRelAddr.count(src) != 0) // is loacl tmp
        reg = allocReg(src, s);
    else if (src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) // is global alloc
    {
        reg = findFreeReg();
        s += make_la(reg, CASTLABEL(src->name));
    }
    else
        assert(false);

    assert(src->ty->tag == KOOPA_RTT_POINTER && src->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY);
    size_t sz = get_type_size(src->ty->data.pointer.base->data.array.base);

    RegNo tmp1 = allocReg(get_elem_ptr.index, s);
    RegNo tmp2 = findFreeReg();
    s += make_li(tmp2, sz);
    s += make_binary_code("mul", tmp1, tmp1, tmp2);
    usedReg[tmp2] = 0;
    s += make_binary_code("add", reg, reg, tmp1);
    usedReg[tmp1] = 0;
    return reg;
}

RegNo genCode(const koopa_raw_get_ptr_t &get_ptr, string &s)
{
    RegNo reg;
    auto src = get_ptr.src;
    if (LocalAllocRelAddr.count(src) != 0) // is local alloc
    {
        reg = findFreeReg();
        s += gen_addi(reg, SP, LocalAllocRelAddr[src]);
    }
    else if (LocalTmpRelAddr.count(src) != 0) // is loacl tmp
        reg = allocReg(src, s);
    // else if (src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) // is global alloc
    // {
    //     reg = findFreeReg();
    //     s += make_la(reg, CASTLABEL(src->name));
    // }
    else
        assert(false);

    // assert(src->ty->tag == KOOPA_RTT_POINTER && src->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY);
    // size_t sz = get_type_size(src->ty->data.pointer.base->data.array.base);
    assert(src->ty->tag == KOOPA_RTT_POINTER);
    size_t sz = get_pointer_size(src->ty);

    RegNo tmp1 = allocReg(get_ptr.index, s);
    RegNo tmp2 = findFreeReg();
    s += make_li(tmp2, sz);
    s += make_binary_code("mul", tmp1, tmp1, tmp2);
    usedReg[tmp2] = 0;
    s += make_binary_code("add", reg, reg, tmp1);
    usedReg[tmp1] = 0;
    return reg;
}

// string genCode(const koopa_raw_binary_t &binary)
// {
//     string s = "";
//     bool limm = 0, rimm = 0;
//     int lreg = allocReg(binary.lhs, limm);
//     int rreg = allocReg(binary.rhs, rimm);
//     if (limm)
//         s += "    li " + regname[lreg] + ", " + to_string(binary.lhs->kind.data.integer.value) + "\n";
//     if (rimm)
//         s += "    li " + regname[rreg] + ", " + to_string(binary.rhs->kind.data.integer.value) + "\n";
//     int freg = findFreeReg();
//     switch (binary.op)
//     {
//     case KOOPA_RBO_EQ:
//         /* code */
//         s += make_binary_code("xor", freg, lreg, rreg);
//         s += "    seqz " + regname[freg] + ", " + regname[freg] + "\n";
//         break;
//     case KOOPA_RBO_SUB:
//         s += make_binary_code("sub", freg, lreg, rreg);
//         break;
//     case KOOPA_RBO_ADD:
//         s += make_binary_code("add", freg, lreg, rreg);
//         break;
//     case KOOPA_RBO_MUL:
//         s += make_binary_code("mul", freg, lreg, rreg);
//         break;
//     case KOOPA_RBO_DIV:
//         s += make_binary_code("div", freg, lreg, rreg);
//         break;
//     case KOOPA_RBO_MOD:
//         s += make_binary_code("rem", freg, lreg, rreg);
//         break;
//     case KOOPA_RBO_LT:
//         s += make_binary_code("slt", freg, lreg, rreg);
//         break;
//     case KOOPA_RBO_GT:
//         s += make_binary_code("sgt", freg, lreg, rreg);
//         break;
//     case KOOPA_RBO_LE:
//         s += make_binary_code("sgt", freg, lreg, rreg);
//         s += "    seqz " + regname[freg] + ", " + regname[freg] + "\n";
//         break;
//     case KOOPA_RBO_GE:
//         s += make_binary_code("slt", freg, lreg, rreg);
//         s += "    seqz " + regname[freg] + ", " + regname[freg] + "\n";
//         break;
//     case KOOPA_RBO_NOT_EQ:
//         s += make_binary_code("xor", freg, lreg, rreg);
//         s += "    snez " + regname[freg] + ", " + regname[freg] + "\n";
//         break;
//     case KOOPA_RBO_AND:
//         s += make_binary_code("and", freg, lreg, rreg);
//         break;
//     case KOOPA_RBO_OR:
//         s += make_binary_code("or", freg, lreg, rreg);
//         break;
//     default:
//         break;
//     }
//     MReg[binary] = freg;
//     // if (limm)
//     usedReg[lreg] = 0;
//     // if (rimm)
//     usedReg[rreg] = 0;

//     return s;
// }
