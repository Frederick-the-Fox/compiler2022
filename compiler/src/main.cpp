#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include "astdefinition.h"
#include "koopa.h"
// #include "irdefinition.h"
#include "gencode.h"

using namespace std;

// 声明 lexer 的输入, 以及 parser 函数
// 为什么不引用 sysy.tab.hpp 呢? 因为首先里面没有 yyin 的定义
// 其次, 因为这个文件不是我们自己写的, 而是被 Bison 生成出来的
// 你的代码编辑器/IDE 很可能找不到这个文件, 然后会给你报错 (虽然编译不会出错)
// 看起来会很烦人, 于是干脆采用这种看起来 dirty 但实际很有效的手段
extern FILE *yyin;
extern int yyparse(unique_ptr<CompUnitAST> &ast);

int main(int argc, const char *argv[])
{
	// 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
	// compiler 模式 输入文件 -o 输出文件
	assert(argc == 5);
	auto mode = argv[1];
	auto input = argv[2];
	auto output = argv[4];

	// 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
	yyin = fopen(input, "r");
	assert(yyin);

	// 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
	unique_ptr<CompUnitAST> ast;
	auto ret = yyparse(ast);
	assert(!ret);

	// 输出解析得到的 AST, 其实就是个字符串
	// ast->Dump();

	FILE *outputfp = freopen(output, "w", stdout);

	auto p = reinterpret_cast<CompUnitAST *>(ast.get());
	clog << "AST finished." << endl;

	// p->Dump();

	auto res = p->SynIR();
	clog << "SynIR finished." << endl;
	string strIR;
	res->Dump(strIR);

	clog << "mode:" << mode << endl;
	if (strcmp(mode, "-koopa") == 0)
		cout << strIR;
	else if (strcmp(mode, "-riscv") == 0)
	{

		koopa_program_t program;
		koopa_error_code_t err_code = koopa_parse_from_string(strIR.c_str(), &program);
		assert(err_code == KOOPA_EC_SUCCESS); // 确保解析时没有出错
		// 创建一个 raw program builder, 用来构建 raw program
		koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
		// 将 Koopa IR 程序转换为 raw program
		koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
		// 释放 Koopa IR 程序占用的内存
		koopa_delete_program(program);

		string strcode = "";
		genCode(raw, strcode);
		cout << strcode;
	}
	// ast->synRawProgram(raw_program); // my own raw program synthesizer
	// ret = koopa_generate_raw_to_koopa(&raw_program, &program);
	// cerr << "generate successfully!" << ' ' << ret << endl;

	// ret = koopa_dump_to_stdout(program);
	// cerr << "dump successfully!" << endl;

	fclose(outputfp);
	return 0;
}
