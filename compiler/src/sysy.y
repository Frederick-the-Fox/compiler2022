%code requires {
  #include <memory>
  #include <string>
  
}

%{

#include <iostream>
#include <memory>
#include <string>
#include "astdefinition.h"

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<CompUnitAST> &ast, const char *s);

using namespace std;

%}

// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<CompUnitAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  CompUnitAST *comp_unit_ast_val;
  FuncDefAST *func_def_val;
  FuncTypeAST *func_type_val;
  BlockAST * block_val;
  BlockItemAST *block_item_val;
  StmtAST* stmt_val;
  NumberAST* number_val;
  ExpAST* exp_val;
  PrimaryExpAST *primary_exp_val;
  UnaryExpAST* unary_exp_val;
  UnaryOpAST* unary_op_val;
  MulExpAST* mul_exp_val;
  AddExpAST* add_exp_val;
  RelExpAST* rel_exp_val;
  EqExpAST* eq_exp_val; 
  LAndExpAST* land_exp_val;
  LOrExpAST* lor_exp_val;
  DeclAST* decl_val;
  ConstDeclAST* const_decl_val;
  ConstDefAST* const_def_val;
  ConstInitValAST* const_init_val_val;
  LValAST* lval_val;
  ConstExpAST* const_exp_val;
  VarDeclAST* var_decl_val;
  VarDefAST* var_def_val;
  InitValAST* init_val_val;
  FuncFParamAST* func_f_param_val;
  DeclPFuncDefAST* decl_p_func_def_val;

  vector<unique_ptr<BlockItemAST> >* block_item_list_val;
  vector<unique_ptr<ConstDefAST> >* const_def_list_val; 
  vector<unique_ptr<VarDefAST> >* var_def_list_val;
  vector<unique_ptr<FuncFParamAST>>* func_f_params_val;
  vector<unique_ptr<ExpAST>>* func_r_params_val;
  vector<unique_ptr<ConstExpAST>>* const_exps_val;
  vector<unique_ptr<ConstInitValAST>>* const_init_vals_val;
  vector<unique_ptr<InitValAST>>* init_vals_val;
  vector<unique_ptr<ExpAST>>* exps_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT VOID RETURN CONST IF ELSE WHILE BREAK CONTINUE 
%token OP_ADD OP_SUB OP_MUL OP_DIV OP_MOD OP_NOT OP_LT OP_GT OP_LE OP_GE OP_EQ OP_NEQ OP_LAND OP_LOR
%token <str_val> IDENT 
%token <int_val> INT_CONST

// 非终结符的类型定义

%type <func_def_val> FuncDef 
%type <func_type_val> FuncType 
%type <block_val> Block 
%type <block_item_val> BlockItem
%type <stmt_val> Stmt ClosedStmt OpenStmt
%type <number_val> Number
%type <exp_val> Exp
%type <primary_exp_val> PrimaryExp
%type <unary_exp_val> UnaryExp
%type <unary_op_val> UnaryOp
%type <mul_exp_val> MulExp
%type <add_exp_val> AddExp
%type <rel_exp_val> RelExp
%type <eq_exp_val> EqExp
%type <land_exp_val> LAndExp
%type <lor_exp_val> LOrExp 
%type <decl_val> Decl
%type <const_decl_val> ConstDecl
%type <const_def_val> ConstDef
%type <const_init_val_val> ConstInitVal
%type <lval_val> LVal
%type <const_exp_val> ConstExp
%type <var_decl_val> VarDecl
%type <var_def_val> VarDef
%type <init_val_val> InitVal
%type <func_f_param_val> FuncFParam


%type <block_item_list_val> BlockItemList
%type <const_def_list_val> ConstDefList
%type <var_def_list_val> VarDefList
%type <func_f_params_val> FuncFParams 
%type <func_r_params_val> FuncRParams
%type <decl_p_func_def_val> DeclPFuncDef
%type <const_exps_val> ConstExps
%type <const_init_vals_val> ConstInitVals
%type <init_vals_val> InitVals
%type <exps_val> Exps

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : DeclPFuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->list.push_back(unique_ptr<DeclPFuncDefAST>($1));
    ast = move(comp_unit);
  }
  | CompUnit DeclPFuncDef {
    ast->list.push_back(unique_ptr<DeclPFuncDefAST>($2));
  }
  ;

DeclPFuncDef 
  : Decl {
    auto ast = new DeclPFuncDefAST();
    ast->prod_id=1;
    ast->decl = unique_ptr<DeclAST>($1);
    $$ = ast;
  }
  | FuncDef {
    auto ast = new DeclPFuncDefAST();
    ast->prod_id=2;
    ast->func_def = unique_ptr<FuncDefAST>($1);
    $$ = ast;
  }
  ;

// CompUnit
//   : FuncDef {  }
//   | CompUnit FuncDef { }
//   ;


// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担
FuncDef
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<FuncTypeAST>((FuncTypeAST *)$1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BlockAST>((BlockAST *)$5);
    $$ = ast;
  }
  | FuncType IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<FuncTypeAST>((FuncTypeAST *)$1);
    ast->ident = *unique_ptr<string>($2);
    ast->func_f_params.swap(*($4));
    ast->block = unique_ptr<BlockAST>((BlockAST *)$6);
    $$ = ast;
  }
  ;

FuncFParams
  : FuncFParam {
    auto list = new vector<unique_ptr<FuncFParamAST>>;
    list->push_back(unique_ptr<FuncFParamAST>($1));
    $$ = list;
  }
  | FuncFParams ',' FuncFParam {
    auto list = $1;
    list->push_back(unique_ptr<FuncFParamAST>($3));
    $$=list;
  }
  ;


// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast -> ty = "int";
	  $$ = ast;
  }
  | VOID {
    auto ast = new FuncTypeAST();
    ast -> ty = "void";
    $$ = ast;
  }
  ;

Block
  : '{' BlockItemList '}' {
	  auto ast = new BlockAST();
    ast->block_item_list.swap(*($2));
    delete($2);
	  $$ = ast;
  }
  ;

BlockItemList
  : /*empty*/ {
      auto list = new vector<unique_ptr<BlockItemAST>>;
      $$ = list;
  }
  | BlockItemList BlockItem {
      auto list = $1;
      list->push_back(unique_ptr<BlockItemAST>($2));
      $$ = list;
  }
  ;

BlockItem
  : Decl {
    auto ast = new BlockItemAST();
    ast->prod_id=1;
    ast->decl = unique_ptr<DeclAST>($1);
    $$=ast;
  } 
  | Stmt {
    auto ast = new BlockItemAST();
    ast->prod_id=2;
    ast->stmt = unique_ptr<StmtAST>($1);
    $$=ast;
  } 
  ;

Stmt
  : OpenStmt {
    $$ = $1;
  }
  | ClosedStmt {
    $$ = $1;
  }
  ;

ClosedStmt
  : LVal '=' Exp ';' {
    auto ast = new StmtAST();
    ast->prod_id=1;
    ast->lval = unique_ptr<LValAST>($1);
    ast->exp = unique_ptr<ExpAST>($3);
    $$=ast;
  }
  | RETURN Exp ';' {
	  auto ast = new StmtAST();
    ast->prod_id=2;
    ast->exp= unique_ptr<ExpAST>((ExpAST *)$2);
    $$ = ast;
  }
  | RETURN ';'
  {
    auto ast = new StmtAST();
    ast->prod_id=2;
    ast->exp= nullptr;
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new StmtAST();
    ast->prod_id=3;
    ast->exp= unique_ptr<ExpAST>((ExpAST *)$1);
    $$ = ast;
  }
  | ';' {
    auto ast = new StmtAST();
    ast->prod_id=3;
    ast->exp=nullptr;
    $$ = ast;
  }
  | Block {
    auto ast = new StmtAST();
    ast->prod_id=4;
    ast->block=unique_ptr<BlockAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' ClosedStmt ELSE ClosedStmt {
    auto ast = new StmtAST();
    ast -> prod_id = 5;
    ast -> exp = unique_ptr<ExpAST>($3);
    ast -> t_stmt = unique_ptr<StmtAST>($5);
    ast -> f_stmt = unique_ptr<StmtAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' ClosedStmt {
    auto ast = new StmtAST();
    ast -> prod_id = 6;
    ast -> exp = unique_ptr<ExpAST>($3);
    ast -> t_stmt = unique_ptr<StmtAST>($5);
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new StmtAST();
    ast -> prod_id = 7;
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new StmtAST();
    ast -> prod_id = 8;
    $$ = ast;
  }
  ;

OpenStmt
  : IF '(' Exp ')' Stmt  {
    auto ast = new StmtAST();
    ast -> prod_id = 5;
    ast -> exp = unique_ptr<ExpAST>($3);
    ast -> t_stmt = unique_ptr<StmtAST>($5);
    ast -> f_stmt = nullptr;
    $$ = ast;
  }
  | IF '(' Exp ')' ClosedStmt ELSE OpenStmt {
    auto ast = new StmtAST();
    ast -> prod_id = 5;
    ast -> exp = unique_ptr<ExpAST>($3);
    ast -> t_stmt = unique_ptr<StmtAST>($5);
    ast -> f_stmt = unique_ptr<StmtAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' OpenStmt {
    auto ast = new StmtAST();
    ast -> prod_id = 6;
    ast -> exp = unique_ptr<ExpAST>($3);
    ast -> t_stmt = unique_ptr<StmtAST>($5);
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
	auto ast = new NumberAST();
	ast->int_const = $1;
	$$ = ast;
  }
  ;

Exp
  : LOrExp {
    auto ast = new ExpAST();
    ast->lor_exp = unique_ptr<LOrExpAST>((LOrExpAST *)$1);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')'
  {
    auto ast = new PrimaryExpAST();
    ast->prod_id = 1;
    ast->exp = unique_ptr<ExpAST>((ExpAST* )$2);
    $$ = ast;
  }
  | Number
  {
    auto ast = new PrimaryExpAST();
    ast->prod_id = 2;
    ast->number = unique_ptr<NumberAST>((NumberAST*)$1);
    $$ = ast;
  }
  | LVal
  {
    auto ast = new PrimaryExpAST();
    ast->prod_id = 3;
    ast->lval = unique_ptr<LValAST>($1);
    $$ = ast;
  }
  ;

UnaryExp
  : PrimaryExp
  {
    auto ast = new UnaryExpAST();
    ast->prod_id = 1;
    ast->primary_exp = unique_ptr<PrimaryExpAST>((PrimaryExpAST*)$1);
    $$ = ast;
  }
  | UnaryOp UnaryExp
  {
    auto ast = new UnaryExpAST();
    ast->prod_id = 2;
    ast->unary_op = unique_ptr<UnaryOpAST>((UnaryOpAST*)$1);
    ast->unary_exp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$2);
    $$ = ast;
  }
  | IDENT '(' ')'
  {
    auto ast = new UnaryExpAST();
    ast -> prod_id = 3;
    ast -> ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '(' FuncRParams ')'
  {
    auto ast = new UnaryExpAST();
    ast -> prod_id = 3;
    ast -> ident = *unique_ptr<string>($1);
    ast -> func_r_params.swap(*($3));
    $$ = ast;  
  }
  ;

FuncRParams
  : Exp {
    auto list = new vector<unique_ptr<ExpAST>>;
    list->push_back(unique_ptr<ExpAST>($1));
    $$ = list;
  }
  | FuncRParams ',' Exp {
    auto list = $1;
    list->push_back(unique_ptr<ExpAST>($3));
    $$=list;
  }
  ;

UnaryOp
  : OP_ADD
  {
    auto ast = new UnaryOpAST();
    ast->prod_id = 1;
    $$ = ast;
  }
  | OP_SUB
  {
    auto ast = new UnaryOpAST();
    ast->prod_id = 2;
    $$ = ast;
  }
  | OP_NOT
  {
    auto ast = new UnaryOpAST();
    ast->prod_id = 3;
    $$ = ast;
  }
  ;

MulExp
  : UnaryExp
  {
    auto ast = new MulExpAST();
    ast->prod_id = 1;
    ast->unary_exp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$1);
    $$ = ast;
  }
  | MulExp OP_MUL UnaryExp
  {
    auto ast = new MulExpAST();
    ast->prod_id = 2;
    ast->op = "*";
    ast->mul_exp=unique_ptr<MulExpAST>((MulExpAST *)$1);
    ast->unary_exp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$3);
    $$ = ast;
  }
  | MulExp OP_DIV UnaryExp
  {
    auto ast = new MulExpAST();
    ast->prod_id = 2;
    ast->op = "/";
    ast->mul_exp=unique_ptr<MulExpAST>((MulExpAST *)$1);
    ast->unary_exp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$3);
    $$ = ast;
  }
  | MulExp OP_MOD UnaryExp
  {
    auto ast = new MulExpAST();
    ast->prod_id = 2;
    ast->op = "%";
    ast->mul_exp=unique_ptr<MulExpAST>((MulExpAST *)$1);
    ast->unary_exp = unique_ptr<UnaryExpAST>((UnaryExpAST*)$3);
    $$ = ast;
  }
  ;

AddExp
  : MulExp
  {
    auto ast = new AddExpAST();
    ast->prod_id = 1;
    ast->mul_exp = unique_ptr<MulExpAST>((MulExpAST*)$1);
    $$ = ast;
  }
  | AddExp OP_ADD MulExp
  {
    auto ast = new AddExpAST();
    ast->prod_id = 2;
    ast->op = "+";
    ast->add_exp=unique_ptr<AddExpAST>((AddExpAST *)$1);
    ast->mul_exp = unique_ptr<MulExpAST>((MulExpAST*)$3);
    $$ = ast;
  }
  | AddExp OP_SUB MulExp
  {
    auto ast = new AddExpAST();
    ast->prod_id = 2;
    ast->op = "-";
    ast->add_exp=unique_ptr<AddExpAST>((AddExpAST *)$1);
    ast->mul_exp = unique_ptr<MulExpAST>((MulExpAST*)$3);
    $$ = ast;
  }
  ;

RelExp
  : AddExp
  {
    auto ast = new RelExpAST();
    ast->prod_id = 1;
    ast->add_exp = unique_ptr<AddExpAST>((AddExpAST*)$1);
    $$ = ast;
  }
  | RelExp OP_LT AddExp
  {
    auto ast = new RelExpAST();
    ast->prod_id = 2;
    ast->op = "<";
    ast->rel_exp=unique_ptr<RelExpAST>((RelExpAST *)$1);
    ast->add_exp = unique_ptr<AddExpAST>((AddExpAST*)$3);
    $$ = ast;
  }
  | RelExp OP_GT AddExp
  {
    auto ast = new RelExpAST();
    ast->prod_id = 2;
    ast->op = ">";
    ast->rel_exp=unique_ptr<RelExpAST>((RelExpAST *)$1);
    ast->add_exp = unique_ptr<AddExpAST>((AddExpAST*)$3);
    $$ = ast;
  }
  | RelExp OP_LE AddExp
  {
    auto ast = new RelExpAST();
    ast->prod_id = 2;
    ast->op = "<=";
    ast->rel_exp=unique_ptr<RelExpAST>((RelExpAST *)$1);
    ast->add_exp = unique_ptr<AddExpAST>((AddExpAST*)$3);
    $$ = ast;
  }
  | RelExp OP_GE AddExp
  {
    auto ast = new RelExpAST();
    ast->prod_id = 2;
    ast->op = ">=";
    ast->rel_exp=unique_ptr<RelExpAST>((RelExpAST *)$1);
    ast->add_exp = unique_ptr<AddExpAST>((AddExpAST*)$3);
    $$ = ast;
  }
  ;


EqExp
  : RelExp
  {
    auto ast = new EqExpAST();
    ast->prod_id = 1;
    ast->rel_exp = unique_ptr<RelExpAST>((RelExpAST*)$1);
    $$ = ast;
  }
  | EqExp OP_EQ RelExp
  {
    auto ast = new EqExpAST();
    ast->prod_id = 2;
    ast->op = "==";
    ast->eq_exp=unique_ptr<EqExpAST>((EqExpAST *)$1);
    ast->rel_exp = unique_ptr<RelExpAST>((RelExpAST*)$3);
    $$ = ast;
  }
  | EqExp OP_NEQ RelExp
  {
    auto ast = new EqExpAST();
    ast->prod_id = 2;
    ast->op = "!=";
    ast->eq_exp=unique_ptr<EqExpAST>((EqExpAST *)$1);
    ast->rel_exp = unique_ptr<RelExpAST>((RelExpAST*)$3);
    $$ = ast;
  }
  ;

LAndExp
  : EqExp
  {
    auto ast = new LAndExpAST();
    ast->prod_id = 1;
    ast->eq_exp = unique_ptr<EqExpAST>((EqExpAST*)$1);
    $$ = ast;
  }
  | LAndExp OP_LAND EqExp
  {
    auto ast = new LAndExpAST();
    ast->prod_id = 2;
    ast->op = "&&";
    ast->land_exp=unique_ptr<LAndExpAST>((LAndExpAST *)$1);
    ast->eq_exp = unique_ptr<EqExpAST>((EqExpAST*)$3);
    $$ = ast;
  }
  ;
LOrExp
  : LAndExp
  {
    auto ast = new LOrExpAST();
    ast->prod_id = 1;
    ast->land_exp = unique_ptr<LAndExpAST>((LAndExpAST*)$1);
    $$ = ast;
  }
  | LOrExp OP_LOR LAndExp
  {
    auto ast = new LOrExpAST();
    ast->prod_id = 2;
    ast->op = "||";
    ast->lor_exp=unique_ptr<LOrExpAST>((LOrExpAST *)$1);
    ast->land_exp = unique_ptr<LAndExpAST>((LAndExpAST*)$3);
    $$ = ast;
  }
  ;

Decl
  : ConstDecl {
    auto ast=new DeclAST();
    ast->prod_id=1;
    ast->const_decl=unique_ptr<ConstDeclAST>($1);
    $$=ast;
  }
  | VarDecl {
    auto ast=new DeclAST();
    ast->prod_id=2;
    ast->var_decl=unique_ptr<VarDeclAST>($1);
    $$=ast;
  }
  ;

ConstDecl
  : CONST FuncType ConstDefList ';' {
    auto ast = new ConstDeclAST();
    ast->b_type="int";
    ast->const_def_list.swap(*($3));
    delete($3);
    $$=ast;
  } 
  ;

ConstDefList
  : ConstDef {
    auto list = new vector<unique_ptr<ConstDefAST>>;
    list->push_back(unique_ptr<ConstDefAST>($1));
    $$ = list;
  }
  | ConstDefList ',' ConstDef {
    auto list = $1;
    list->push_back(unique_ptr<ConstDefAST>($3));
    $$=list;
  }
  ;

ConstDef
  : IDENT ConstExps '=' ConstInitVal {
    auto ast = new ConstDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->const_exps.swap(*($2));
    ast->const_init_val= unique_ptr<ConstInitValAST>($4);
    $$ = ast;
  }
  ;

ConstExps 
  : /* empty */ {
    auto list = new vector<unique_ptr<ConstExpAST>>;
    $$ = list;
  }
  | ConstExps '[' ConstExp ']' {
    auto list = $1;
    list->push_back(unique_ptr<ConstExpAST>($3));
    $$ = list;
  }
  ;


ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
    ast->prod_id = 1;
    ast->const_exp = unique_ptr<ConstExpAST>($1);
    $$=ast;
  }
  | '{' ConstInitVals '}' {
    auto ast = new ConstInitValAST();
    ast->prod_id = 2;
    ast->const_init_vals.swap(*($2));
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new ConstInitValAST();
    ast->prod_id = 2;
    $$ = ast;
  }
  ;

ConstInitVals 
  : ConstInitVal {
    auto list = new vector<unique_ptr<ConstInitValAST>>;
    list->push_back(unique_ptr<ConstInitValAST>($1));
    $$ = list;
  }
  | ConstInitVals ',' ConstInitVal {
    auto list = $1;
    list->push_back(unique_ptr<ConstInitValAST>($3));
    $$=list;
  }
  ;

LVal
  : IDENT Exps {
    auto ast = new LValAST();
    ast->ident = *unique_ptr<string>($1);
    ast->exps.swap(*($2));
    $$ = ast;
  }
  ;

Exps 
  : /* empty */ {
    auto list = new vector<unique_ptr<ExpAST>>;
    $$ = list;
  }
  | Exps '[' Exp ']' {
    auto list = $1;
    list->push_back(unique_ptr<ExpAST>($3));
    $$ = list;
  }
  ;


ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp = unique_ptr<ExpAST>($1);
    $$ = ast;
  }
  ;

VarDecl
  : FuncType VarDefList ';'{
    auto ast = new VarDeclAST();
    ast->b_type="int";
    ast->var_def_list.swap(*($2));
    delete($2);
    $$=ast;
  }
  ;

VarDefList
  : VarDef {
    auto list = new vector<unique_ptr<VarDefAST>>;
    list->push_back(unique_ptr<VarDefAST>($1));
    $$ = list;
  }
  | VarDefList ',' VarDef {
    auto list = $1;
    list->push_back(unique_ptr<VarDefAST>($3));
    $$=list;
  }
  ;

VarDef 
  : IDENT ConstExps {
    auto ast = new VarDefAST();
    ast->prod_id=1;
    ast->ident=*unique_ptr<string>($1);
    ast->const_exps.swap(*($2)); 
    $$=ast;
  }
  | IDENT ConstExps '=' InitVal {
    auto ast = new VarDefAST();
    ast->prod_id=2;
    ast->ident=*unique_ptr<string>($1);
    ast->const_exps.swap(*($2)); 
    ast->init_val = unique_ptr<InitValAST>($4);
    $$=ast;
  }

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->prod_id=1;
    ast->exp = unique_ptr<ExpAST>($1);
    $$=ast;
  }
  | '{' InitVals '}' {
    auto ast = new InitValAST();
    ast->prod_id=2;
    ast->init_vals.swap(*($2));
    $$ = ast;
  }
  | '{' '}' {
    auto ast = new InitValAST();
    ast->prod_id=2;
    $$ = ast;
  }
  ;

InitVals
  : InitVal {
    auto list = new vector<unique_ptr<InitValAST>>;
    list->push_back(unique_ptr<InitValAST>($1));
    $$ = list;
  }
  | InitVals ',' InitVal {
    auto list = $1;
    list->push_back(unique_ptr<InitValAST>($3));
    $$=list;
  }
  ;

FuncFParam
  : FuncType IDENT {
    auto ast = new FuncFParamAST();
    ast->prod_id=1;
    ast->b_type = "int";
    ast->ident = *unique_ptr<string>($2);
    $$ = ast;
  }
  | FuncType IDENT '[' ']' ConstExps {
    auto ast = new FuncFParamAST();
    ast->prod_id = 2;
    ast->b_type = "int";
    ast->ident = *unique_ptr<string>($2);
    ast->const_exps.swap(*($5));
    $$ = ast;
  }
  ;

%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
void yyerror(unique_ptr<CompUnitAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
