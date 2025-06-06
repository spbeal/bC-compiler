%define api.pure false
%define parse.error verbose
%defines
%locations
// top two lines fixes local machine seg faults



%{
#include "scanType.h"
#include "yyerror.h"

#include "treeNodes.h"
#include "treeUtils.h"

#include "semantics.h"
#include "emitcode.h"
#include "codegen.h"

using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

extern void yyerror(const char *msg);

//TreeNode* var = newDeclNode(VarK, UndefinedType, $2);
void printToken(TokenData myData, string tokenName, int type = 0) {
   cout << "Line: " << myData.linenum << " Type: " << tokenName;
   if(type==0)
     cout << " Token: " << myData.tokenstr;
   if(type==1)
     cout << " Token: " << myData.nvalue;
   if(type==2)
     cout << " Token: " << myData.cvalue;
   cout << endl;
}
TreeNode *syntaxTree;
SymbolTable * symtab;

void initAll()
{
  syntaxTree = initializeNode(NULL, NULL, NULL, NULL);
  initErrorProcessing();
  initTokenStrings();
}
  
extern "C" int tokenErrors;
int numErrors = 0;
int numWarnings = 0;
extern int line;
extern int yylex();

%}

%union
{
   TokenData * tokenData;
   TreeNode * tree;
   ExpType type; 
}

%type   <tree> program 
%type   <tree> precomList declList 
%type   <tree> decl localDecls varDecl varDeclId scopedVarDecl varDeclList varDeclInit 
%type   <tree> funDecl
%type   <tree> parms parmList parmTypeList parmIdList parmId
%type   <tree> stmt expStmt compoundStmt stmtList returnStmt breakStmt
%type   <tree> factor 
%type   <tree> matched unmatched 
%type   <tree> mutable immutable
%type   <tree> call args argList
%type   <tree> iterRange
%type   <tree> exp simpleExp andExp unaryRelExp relExp minmaxExp sumExp mulExp unaryExp
%type   <tree> constant 

%type <type> typeSpec

%token <tokenData> FIRSTOP
%token <tokenData> '(' ')' ',' ';' '[' '{' '}' ']' ':' '?' '<' '>' '=' '-' '+' '*' '/' '%' 
%token   <tokenData>  OP
%token   <tokenData>  AND NOT OR
%token   <tokenData>  PRECOMPILER
%token   <tokenData>  EQ LEQ GEQ NEQ
%token   <tokenData>  MAX MIN
%token   <tokenData>  ADDASS SUBASS MULASS DIVASS
%token   <tokenData>  DEC INC 
%token   <tokenData>  SIZEOF

%token   <tokenData>  INT 
%token   <tokenData>  BOOL
%token   <tokenData>  BOOLCONST NUMCONST
%token   <tokenData>  CHARCONST STRINGCONST

%token   <tokenData>  CHAR
%token   <tokenData>  ID
%token   <tokenData>  RETURN
%token   <tokenData>  BREAK
%token   <tokenData>  ERROR 

%token   <tokenData>  THEN IF ELSE
%token   <tokenData>  FOR BY TO
%token   <tokenData>  WHILE DO 
%token   <tokenData>  STATIC
// %token   <tokenData>  COMMENT
%token   <tokenData>  CHSIGN
%token <tokenData> LASTOP

%type   <tokenData> assignop relop sumop mulop minmaxop unaryop

%token <tokenData> LASTTERM

%%
program : precomList declList {syntaxTree = $2;}
    ;
    
precomList : precomList PRECOMPILER {$$ = $1; printf("%s\n", yylval.tokenData->tokenstr);}
    | PRECOMPILER                   {$$=NULL; printf("%s\n", yylval.tokenData->tokenstr);}
    | /* empty */                   {$$=NULL;}
    ;

declList : declList decl {$$ = addSibling($1, $2);}
    | decl      {$$ = $1;}
    ;

decl : varDecl  {$$ = $1;}
    | funDecl   {$$ = $1;}
    | error   {$$ = NULL; yyerrok;}
    ;

varDecl : typeSpec varDeclList ';' {setType($2, $1, false); $$ = $2; }
    | error varDeclList ';' {$$ = NULL; yyerrok;}
    | typeSpec error ';'   {$$ = NULL; yyerrok;}
    ;

scopedVarDecl : STATIC typeSpec varDeclList ';' {$$ = $3; setType($3, $2, true);}
    | typeSpec varDeclList ';' {$$ = $2; setType($2, $1, false) ;}
    | typeSpec error ';'   {$$ = NULL; yyerrok;}
    | STATIC typeSpec ';' {$$ = NULL; yyerrok;}
    ;

varDeclList : varDeclList ',' varDeclInit {$$ = addSibling($1, $3);}
    | varDeclList ',' error   {$$ = NULL;}
    | varDeclInit {$$ = $1;}
    | error   {$$ = NULL;}
    ;

varDeclInit : varDeclId { $$ = $1;}
    | varDeclId ':' simpleExp {$$ = $1; if ($$ != NULL) $$->child[0] = $3;}
    | error ':' simpleExp  {$$ = NULL; yyerrok;}
    | varDeclId ':' error   {$$ = NULL; yyerrok;}
    ;

varDeclId : ID {$$ = newDeclNode(VarK, UndefinedType, $1);}
    | ID '[' NUMCONST ']' {$$ = newDeclNode(VarK, UndefinedType, $1); $$->isArray = true; $$->size = $3->nvalue + 1;}
    | ID '[' error   {$$ = NULL; yyerrok;}
    | error ']' {$$ = NULL; yyerrok;}
    ;

typeSpec : INT {$$ = Integer;}
    | BOOL {$$ = Boolean;}
    | CHAR {$$ = Char;}
    ;

funDecl : typeSpec ID '(' parms ')' stmt { $$ = newDeclNode(FuncK, $1, $2, $4, $6);}
    | ID '(' parms ')' stmt {$$ = newDeclNode(FuncK, Void, $1, $3, $5);}
    | typeSpec error   {$$ = NULL; }
    | typeSpec ID '(' error   {$$ = NULL; }
    | typeSpec ID '(' parms ')' error {$$ = NULL; }
    | ID '(' error  {$$ = NULL; }
    | ID '('parms')' error {$$ = NULL;}
    ;

parms : parmList { $$ = $1;}
    |  {$$ = NULL;}
    ;

parmList : parmList ';' parmTypeList { $$ = addSibling($1, $3);}
    | parmTypeList { $$ = $1;}
    | parmList ';' error { $$ = NULL; }
    | error { $$ = NULL; }
    ;

parmTypeList : typeSpec parmIdList { setType($2, $1, false); $$ = $2;}
    | typeSpec error { $$ = NULL; yyerrok;}
    ;

parmIdList: parmIdList ',' parmId {$$ = addSibling($1, $3);}
    | parmId { $$ = $1;}
    | error {$$ = NULL; yyerrok;}
    ;

parmId: ID {$$ = newDeclNode(ParamK, UndefinedType, $1);}
    | ID '[' ']' {$$ = newDeclNode(ParamK, UndefinedType, $1); $$->isArray = true;}
    | error ']' {$$ = NULL; yyerrok;}
    ;

stmt : matched {$$ = $1;}
    | unmatched {$$ = $1;}
    ;

matched : IF simpleExp THEN matched ELSE matched { $$ = newStmtNode(IfK, $1, $2, $4, $6);}
    | IF error THEN matched ELSE unmatched { $$ = NULL; yyerrok;}
    | IF error THEN matched ELSE matched { $$ = NULL; yyerrok;}
    | IF error { $$ = NULL; yyerrok;}
    | IF error ELSE matched { $$ = NULL; yyerrok;}
    | WHILE simpleExp DO matched { $$ = newStmtNode(WhileK, $1, $2, $4);}
    | FOR ID '=' iterRange DO matched 
      { $$ = newStmtNode(ForK, $1, NULL, $4, $6); 
        $$->child[0] = newDeclNode(VarK, Integer, $2); }
    | expStmt { $$ = $1;}
    | compoundStmt { $$ = $1;}
    | returnStmt { $$ = $1;}
    | breakStmt { $$ = $1;}
    ;

iterRange : simpleExp TO simpleExp {$$ = newStmtNode(RangeK, $2, $1, $3);}
    | simpleExp TO simpleExp BY simpleExp {$$ = newStmtNode(RangeK, $2, $1, $3, $5);}
    | simpleExp TO simpleExp BY error {$$ = NULL; yyerrok;}
    | simpleExp TO error {$$ = NULL; yyerrok;}
    | error BY error {$$ = NULL; yyerrok;}
    ;

unmatched  : IF simpleExp THEN stmt {$$ = newStmtNode(IfK, $1, $2, $4);}
    | IF error THEN stmt {$$ = NULL; yyerrok;}
    | IF simpleExp THEN matched ELSE unmatched {$$ = newStmtNode(IfK, $1, $2, $4, $6);}
    | WHILE simpleExp DO unmatched {$$ = newStmtNode(WhileK, $1, $2, $4);}
    | FOR ID '=' iterRange DO unmatched {
        $$ = newStmtNode(ForK, $1, NULL, $4, $6); 
        $$->child[0] = newDeclNode(VarK, Integer, $2);
    }
    ;

expStmt    : exp ';'  {$$ = $1;}
            | error ';' {$$ = NULL; yyerrok;}            
            | ';' {$$ = NULL; yyerrok;}                       
           ;

compoundStmt : '{' localDecls stmtList '}'      {$$ = newStmtNode(CompoundK, $1, $2, $3);}
    ;

localDecls : localDecls scopedVarDecl    {$$ = addSibling($1, $2);}         
             | /* empty */      {$$ = NULL;}                        
             ;

stmtList : stmtList stmt    {$$ = ($2==NULL ? $1 : addSibling($1, $2)); }
    |           {$$ = NULL;}
    ;

returnStmt : RETURN ';'      {$$ = newStmtNode(ReturnK, $1);}                           
             | RETURN exp ';'    {$$ = newStmtNode(ReturnK, $1, $2);}                            
           ;

breakStmt  : BREAK ';'               {$$ = newStmtNode(BreakK, $1);}           
           ;

exp        : mutable assignop exp {$$ = newExpNode(AssignK, $2, $1, $3);}
           | mutable INC {$$ = newExpNode(AssignK, $2, $1);}
           | mutable DEC {$$ = newExpNode(AssignK, $2, $1);}
           | simpleExp {$$ = $1;}
           | error assignop exp {$$ = NULL; yyerrok;}
           | mutable assignop error {$$ = NULL; yyerrok;}
           | error INC {$$ = NULL; yyerrok;}
           | error DEC {$$ = NULL; yyerrok;}
           ;

assignop  : '=' {$$ = $1; }
            | ADDASS {$$ = $1;}
            | SUBASS {$$ = $1;}
            | MULASS {$$ = $1;}
            | DIVASS {$$ = $1;}
          ;

simpleExp  : simpleExp OR andExp {$$ = newExpNode(OpK, $2, $1, $3);}
    | andExp {$$ = $1;}
    | simpleExp OR error {$$ = NULL; yyerrok;}
    ;

andExp     : andExp AND unaryRelExp {$$ = newExpNode(OpK, $2, $1, $3);}
    | unaryRelExp {$$ = $1;}
    | andExp AND error {$$ = NULL; yyerrok;}
    ;

unaryRelExp : NOT unaryRelExp {$$ = newExpNode(OpK, $1, $2);}
    | relExp {$$ = $1;}
    | NOT error {$$ = NULL;}
    ;

relExp     : minmaxExp relop minmaxExp    {$$ = newExpNode(OpK, $2, $1, $3);}            
             | minmaxExp                    {$$ = $1;}
           ;

relop      : LEQ {$$ = $1;}
             | '<' {$$ = $1;}
             | '>' {$$ = $1;}
             | GEQ {$$ = $1;}
             | EQ {$$ = $1;}
             | NEQ {$$ = $1;}
           ;

minmaxExp  : minmaxExp minmaxop sumExp  {$$ = newExpNode(OpK, $2, $1, $3);}            
             | sumExp  {$$ = $1;}
           ;

minmaxop   : MAX {$$ = $1;}
             | MIN {$$ = $1;}
           ;

sumExp     : sumExp sumop mulExp  {$$ = newExpNode(OpK, $2, $1, $3);}            
             | mulExp {$$ = $1;}
             | sumExp sumop error {$$ = NULL; yyerrok;}
           ;

sumop      : '+' {$$ = $1;}
             | '-' {$$ = $1;}
           ;

mulExp     : mulExp mulop unaryExp  {$$ = newExpNode(OpK, $2, $1, $3);}         
             | unaryExp  {$$ = $1;}
             | mulExp mulop error {$$ = NULL; yyerrok;}
           ;

mulop      : '*' {$$ = $1; }
             | '/' {$$ = $1; }
             | '%' {$$ = $1;}
           ;

unaryExp   : unaryop unaryExp {$$ = newExpNode(OpK, $1, $2);}
    | factor {$$ = $1;}
    | unaryop error {$$ = NULL; yyerrok;}
    ;

unaryop    : '-'        {$$ = $1; $$->tokenclass = CHSIGN; $$->tokenstr = strdup("chsign"); }                                     
             | '*'  {$$ = $1; $$->tokenclass = SIZEOF; $$->tokenstr = strdup("sizeof");}                                      
             | '?'  {$$ = $1;}
             ;
factor     : immutable {$$ = $1;}
             | mutable {$$ = $1;}
           ;

mutable    : ID    {$$ = newExpNode(IdK, $1); $$->attr.name = $1->svalue;}                                   
             | ID '[' exp ']'   {
                                 TreeNode * id = newExpNode(IdK, $1);
                                 id->isArray = true;
                                 $$ = newExpNode(OpK, $2, id, $3); 
                                 $$->isArray = true;
                                }                       
           ;

immutable   : '(' exp ')' {$$ = $2;}
            | call {$$ = $1;}
            | constant {$$ = $1;}
            | '(' error {$$ = NULL; yyerrok;}
            | error '(' {$$ = NULL; yyerrok;}
            ;

call       : ID '(' args ')'    {$$ = newExpNode(CallK, $1, $3); $$->attr.name = $1->svalue;}                   
           ;

args       : argList  {$$ = $1;}
             | /* empty */         {$$ = NULL;}                       
           ;

argList    : argList ',' exp  {$$ = ($3==NULL ? $1 : addSibling($1, $3));}                     
             | exp      {$$ = $1;}
           ;

constant   : NUMCONST       {$$ = newExpNode(ConstantK, $1); $$->type = Integer; }      
             | CHARCONST    {$$ = newExpNode(ConstantK, $1); $$->type = Char;}                       
             | STRINGCONST  {
                             $$ = newExpNode(ConstantK, $1); 
                             $$->isArray = true;
                             $$->type = Char; 
                             $$->size = 1 + $1->nvalue; 
                            }                               
             | BOOLCONST    {$$ = newExpNode(ConstantK, $1); $$->type = Boolean;}                              
           ;
%%

int main(int argc, char **argv) {
  int option, index;
  char *file = NULL;
  extern FILE *yyin;
  int globalOffset;
  symtab = new SymbolTable();
  symtab->debug(false);
  initAll();
 
  while ((option = getopt (argc, argv, "")) != -1)
    switch (option)
    {
    default:
        ;
    }
  if ( optind == argc ) yyparse();
  for (index = optind; index < argc; index++) 
  {
    yyin = fopen (argv[index], "r");
    yyparse();
    fclose (yyin);
  }
  if(numErrors == 0)
  {
    syntaxTree = semanticAnalysis(syntaxTree, symtab, globalOffset);
    //printTree(stdout, syntaxTree); // set to true, true for assignment 4
  }
  if(numErrors == 0)
  {
    codegen(stdout, (char *)argv[1], syntaxTree, symtab, globalOffset, false);
  }

  printf("Number of warnings: %d\n", numWarnings);
  printf("Number of errors: %d\n", numErrors);
   
  return 0;
}

