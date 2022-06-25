#include "irdefinition.h"

using namespace std;
void KoopaProgram::Dump(string &s) const
{
    s = "decl @getint(): i32\ndecl @getch(): i32\ndecl @getarray(*i32): i32\ndecl @putint(i32)\ndecl @putch(i32)\ndecl @putarray(i32, *i32)\ndecl @starttime()\ndecl @stoptime()\n\n";
    for (auto iter = global_symbol_def_list.begin(); iter != global_symbol_def_list.end(); iter++)
        (*iter)->Dump(s);
    s += "\n";
    for (auto iter = fun_def_list.begin(); iter != fun_def_list.end(); iter++)
        (*iter)->Dump(s);
}

void KoopaFunDef::Dump(string &s) const
{
    // clog << "KoopaFunDef::Dump() " << symbol << endl;
    s += "fun " + symbol + '(';
    fun_params->Dump(s);
    s += ')';
    if (type != nullptr)
    {
        s += ": ";
        type->Dump(s);
    }
    s += " {\n";
    fun_body->Dump(s);
    s += "}\n\n";
}

void KoopaFunBody::Dump(string &s) const
{
    for (auto iter = block_list.begin(); iter != block_list.end(); iter++)
        (*iter)->Dump(s);
}

void KoopaBlock::Dump(string &s) const
{
    s += block_name + ":\n";
    // clog << "KoopaBlock::Dump() " << block_name << endl;
    for (auto iter = stmt_list.begin(); iter != stmt_list.end(); iter++)
        (*iter)->Dump(s);
    assert(end_stmt != nullptr);
    end_stmt->Dump(s);
}

void KoopaStmt::Dump(string &s) const
{
    switch (tag)
    {
    case KP_ST_SYMBOL_DEF:
        symbol_def->Dump(s);
        break;
    case KP_ST_STORE:
        store->Dump(s);
        break;
    case KP_ST_FUN_CALL:
        s += "    ";
        fun_call->Dump(s);
        s += "\n";
        break;
    default:
        assert(false);
    }
}

void KoopaVal::Dump(string &s) const
{
    if (tag == KP_VAL_INT)
        s += to_string(data);
    else
        s += symbol;
}

void KoopaSymbolDef::Dump(string &s) const
{
    // clog << "KoopaSymbolDef::Dump()" << endl;
    s += "    " + symbol + " = ";
    switch (tag)
    {
    case KP_SD_BINARY_EXPR:
        binary_expr->Dump(s);
        break;
    case KP_SD_MEMORY_DECLARATION:
        memory_declaration->Dump(s);
        break;
    case KP_SD_LOAD:
        load->Dump(s);
        break;
    case KP_SD_FUN_CALL:
        fun_call->Dump(s);
        break;
    case KP_SD_GET_ELEMENT_POINTER:
        get_element_pointer->Dump(s);
        break;
    case KP_SD_GET_POINTER:
        get_pointer->Dump(s);
        break;
    default:
        assert(false);
    }
    s += "\n";
}

void KoopaBinaryExpr::Dump(string &s) const
{
    s += this->binary_op + " ";
    val0->Dump(s);
    s += ", ";
    val1->Dump(s);
}

void KoopaEndStmt::Dump(string &s) const
{
    // clog << "KoopaEndStmt::Dump()" << endl;
    switch (tag)
    {
    case KP_EST_RETURN:
        Return->Dump(s);
        break;
    case KP_EST_BRANCH:
        branch->Dump(s);
        break;
    case KP_EST_JUMP:
        jump->Dump(s);
        break;
    default:
        assert(false);
    }
}

void KoopaReturn::Dump(string &s) const
{
    s += "    ret ";
    if (val != nullptr)
        val->Dump(s);
    s += "\n";
}

void KoopaMemoryDeclaration::Dump(string &s) const
{
    s += "alloc ";
    ty->Dump(s);
}

void KoopaType::Dump(string &s) const
{
    switch (tag)
    {
    case KP_TY_I32:
        s += "i32";
        break;
    case KP_TY_ARRAY_TYPE:
        array_type->Dump(s);
        break;
    case KP_TY_POINTER_TYPE:
        pointer_type->Dump(s);
        break;
    default:
        assert(false);
    }
}

void KoopaArrayType::Dump(string &s) const
{
    s += "[";
    type->Dump(s);
    s += ", " + to_string(data) + "]";
}

void KoopaStore::Dump(string &s) const
{
    s += "    store ";
    val->Dump(s);
    s += ", " + symbol + "\n";
}

void KoopaLoad::Dump(string &s) const
{
    s += "load " + symbol;
}

void KoopaBranch::Dump(string &s) const
{
    s += "    br ";
    val->Dump(s);
    s += ", " + symbol0 + ", " + symbol1 + "\n";
}

void KoopaJump::Dump(string &s) const
{
    s += "    jump " + symbol + "\n";
}

void FunParam::Dump(string &s) const
{
    s += symbol + ": ";
    type->Dump(s);
}

void KoopaFunParams::Dump(string &s) const
{
    if (!list.empty())
    {
        list[0].Dump(s);
        for (size_t i = 1, sz = list.size(); i < sz; ++i)
        {
            s += ", ";
            list[i].Dump(s);
        }
    }
}

void KoopaFunCall::Dump(string &s) const
{
    s += "call " + symbol + "(";
    if (!value_list.empty())
    {
        value_list[0]->Dump(s);
        for (size_t i = 1, sz = value_list.size(); i < sz; ++i)
        {
            s += ", ";
            value_list[i]->Dump(s);
        }
    }
    s += ")";
}

void KoopaGlobalSymbolDef::Dump(string &s) const
{
    s += "global " + symbol + " = ";
    global_memory_declaration->Dump(s);
    s += "\n";
}

void KoopaGlobalMemoryDeclaration::Dump(string &s) const
{
    s += "alloc ";
    type->Dump(s);
    s += ", ";
    initializer->Dump(s);
}

void KoopaInitializer::Dump(string &s) const
{
    switch (tag)
    {
    case KP_INIT_INT:
        s += to_string(data);
        break;
    case KP_INIT_ZEROINIT:
        s += "zeroinit";
        break;
    case KP_INIT_AGGREGATE:
        aggregate->Dump(s);
        break;
    default:
        assert(false);
    }
}

void KoopaGetElementPointer::Dump(string &s) const
{
    s += "getelemptr " + symbol + ", ";
    val->Dump(s);
}

void KoopaGetPointer::Dump(string &s) const
{
    s += "getptr " + symbol + ", ";
    val->Dump(s);
}

void KoopaAggregate::Dump(string &s) const
{
    s += "{";
    initializers[0]->Dump(s);
    for (size_t i = 1, sz = initializers.size(); i < sz; ++i)
    {
        s += ", ";
        initializers[i]->Dump(s);
    }
    s += "}";
}

void KoopaPointerType::Dump(string &s) const
{
    s += "*";
    type->Dump(s);
}