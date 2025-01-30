%{
#include "scanType.h"
#include "treeNodes.h"
#include "treeUtils.h"

using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

void yyerror(const char *msg);
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

void initTree()
{
  syntaxTree = NULL;
}

int numErrors = 0;
int numWarnings = 0;
// the syntax tree goes here
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
%token   <tokenData>  NEQ AND NOT OR
%token   <tokenData>  PRECOMPILER
%token   <tokenData>  NUMCONST
%token   <tokenData>  EQ LEQ GEQ 
%token   <tokenData>  MAX MIN
%token   <tokenData>  ADDASS SUBASS MULASS DIVASS
%token   <tokenData>  DEC INC 
%token   <tokenData>  SIZEOF

%token   <tokenData>  INT 
%token   <tokenData>  BOOL
%token   <tokenData>  BOOLCONST
%token   <tokenData>  CHAR
%token   <tokenData>  ID
%token   <tokenData>  RETURN
%token   <tokenData>  BREAK
%token   <tokenData>  ERROR 

%token   <tokenData>  THEN IF ELSE
%token   <tokenData>  FOR BY TO
%token   <tokenData>  WHILE DO 
%token   <tokenData>  STATIC
%token   <tokenData>  COMMENT
%token   <tokenData>  CHARCONST STRINGCONST
%token   <tokenData>  CHSIGN
%token <tokenData> LASTOP

%type   <tokenData> assignop relop sumop mulop minmaxop unaryop

%token <tokenData> LASTTERM

%%
program : precomList declList {syntaxTree = $2;}
    ;
    
precomList : precomList PRECOMPILER {$$ = NULL; printf("%s\n", yylval.tokenData->tokenstr);}
    | PRECOMPILER                   {$$=NULL; printf("%s\n", yylval.tokenData->tokenstr);}
    | /* empty */                   {$$=NULL;}
    ;

declList : declList decl {$$ = addSibling($1, $2);}
    | decl      {$$ = $1;}
    ;

decl : varDecl  {$$=$1;}
    | funDecl   {$$=$1;}
    ;

varDecl : typeSpec varDeclList ';' {setType($2, $1, false); $$ = $2; }
    ;

scopedVarDecl : STATIC typeSpec varDeclList ';' {$$ = $3; setType($3, $2, true);}
    | typeSpec varDeclList ';' {$$ = $2; setType($2, $1, false) ;}
    ;

varDeclList : varDeclList ',' varDeclInit {$$ = addSibling($1, $3);}
    | varDeclInit {$$ = $1;}
    ;

varDeclInit : varDeclId { $$ = $1;}
    | varDeclId ':' simpleExp {$$ = $1; if ($$ != NULL) $$->child[0] = $3;}
    ;

varDeclId : ID {$$ = newDeclNode(VarK, UndefinedType, $1);}
    | ID '[' NUMCONST ']' {$$ = newDeclNode(VarK, UndefinedType, $1); $$->isArray = true; $$->size = $3->nvalue + 1;}
    ;

typeSpec : INT {$$ = Integer;}
    | BOOL {$$ = Boolean;}
    | CHAR {$$ = Char;}
    ;

funDecl : typeSpec ID '(' parms ')' stmt {$$ = newDeclNode(FuncK, $1, $2, $4, $6);}
    | ID '(' parms ')' stmt {$$ = newDeclNode(FuncK, Void, $1, $3, $5);}
    ;

parms : parmList {$$ = $1;}
    |  {$$ = NULL;}
    ;

parmList : parmList ';' parmTypeList {$$ = addSibling($1, $3);}
    | parmTypeList {$$ = $1;}
    ;

parmTypeList : typeSpec parmIdList {setType($2, $1, false); $$ = $2;}
    ;

parmIdList: parmIdList ',' parmId {$$ = addSibling($1, $3);}
    | parmId {$$ = $1;}
    ;

parmId: ID {$$ = newDeclNode(ParamK, UndefinedType, $1);}
    | ID '[' ']' {$$ = newDeclNode(ParamK, UndefinedType, $1); $$->isArray = true;}
    ;

stmt : matched {$$ = $1;}
    | unmatched {$$ = $1;}
    ;

matched : IF simpleExp THEN matched ELSE matched { $$ = newStmtNode(IfK, $1, $2, $4, $6);}
    | WHILE simpleExp DO matched { $$ = newStmtNode(WhileK, $1, $2, $4);}
    | FOR ID '=' iterRange DO matched { $$ = newStmtNode(ForK, $1, NULL, $4, $6);}
    | expStmt { $$ = $1;}
    | compoundStmt { $$ = $1;}
    | returnStmt { $$ = $1;}
    | breakStmt { $$ = $1;}
    ;

iterRange : simpleExp TO simpleExp {$$ = newStmtNode(RangeK, $2, $1, $3);}
    | simpleExp TO simpleExp BY simpleExp {$$ = NULL;}
    ;

unmatched  : IF simpleExp THEN stmt             {$$ = newStmtNode(IfK, $1, $2, $4);}        
             | IF simpleExp THEN matched ELSE unmatched {$$ = newStmtNode(IfK, $1, $2, $4, $6);} 
             | WHILE simpleExp DO unmatched     {$$ = newStmtNode(WhileK, $1, $2, $4);}          
             | FOR ID '=' iterRange DO unmatched {$$ = newStmtNode(ForK, $1, NULL, $4, $6); newDeclNode(VarK, Integer, $2);}      
           ;

expStmt    : exp ';'  {$$ = $1;}
             | ';'    {$$ = NULL;}                                   
           ;

compoundStmt : '{' localDecls stmtList '}'      {$$ = newStmtNode(CompoundK, $1, $2, $3); yyerrok;}
    ;

localDecls : localDecls scopedVarDecl    {$$ = NULL;}         
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

exp        : mutable assignop exp     {$$ = NULL;}                  
             | mutable INC            {$$ = newExpNode(AssignK, $2, $1);}                 
             | mutable DEC            {$$ = newExpNode(AssignK, $2, $1);}              
             | simpleExp              {$$ = $1;}
           ;

assignop  : '=' {$$ = NULL;}
            | ADDASS {$$ = NULL;}
            | SUBASS {$$ = NULL;}
            | MULASS {$$ = NULL;}
            | DIVASS {$$ = NULL;}
          ;

simpleExp  : simpleExp OR andExp {$$ = newExpNode(OpK, $2, $1, $3);}            
             | andExp {$$ = $1;}
           ;

andExp     : andExp AND unaryRelExp  {$$ = newExpNode(OpK, $2, $1, $3);}                      
             | unaryRelExp          {$$ = $1;}
           ;

unaryRelExp : NOT unaryRelExp     {$$ = newExpNode(OpK, $1, $2);}                        
              | relExp              {$$ = $1;}
            ;

relExp     : minmaxExp relop minmaxExp    {$$ = NULL;}            
             | minmaxExp                    {$$ = $1;}
           ;

relop      : LEQ {$$ = NULL;}
             | '<' {$$ = NULL;}
             | '>' {$$ = NULL;}
             | GEQ {$$ = NULL;}
             | EQ {$$ = NULL;}
             | NEQ {$$ = NULL;}
           ;

minmaxExp  : minmaxExp minmaxop sumExp  {$$ = NULL;}            
             | sumExp  {$$ = NULL;}
           ;

minmaxop   : MAX {$$ = NULL;}
             | MIN {$$ = NULL;}
           ;

sumExp     : sumExp sumop mulExp  {$$ = NULL;}            
             | mulExp {$$ = NULL;}
           ;

sumop      : '+' {$$ = NULL;}
             | '-' {$$ = NULL;}
           ;

mulExp     : mulExp mulop unaryExp  {$$ = NULL;}         
             | unaryExp  {$$ = NULL;}
           ;

mulop      : '*' {$$ = NULL;}
             | '/' {$$ = NULL;}
             | '%' {$$ = NULL;}
           ;

unaryExp   : unaryop unaryExp  {$$ = NULL;}                 
             | factor   {$$ = NULL;}
           ;

unaryop    : '-'        {$$ = $1; $$->tokenclass = CHSIGN; $$->tokenstr = (char *)"chsign";}                                     
             | '*'  {$$ = $1; $$->tokenclass = SIZEOF; $$->tokenstr = (char *)"sizeof";}                                      
             | '?'  {$$ = $1;}
             ;
factor     : immutable {$$ = $1;}
             | mutable {$$ = $1;}
           ;

mutable    : ID    {$$ = newExpNode(IdK, $1);}                                   
             | ID '[' exp ']'   {$$ = newExpNode(IdK, $1, $3);}                       
           ;

immutable  : '(' exp ')'    {$$ = NULL;}                        
             | call         {$$ = $1;}
             | constant     {$$ = $1;}
           ;

call       : ID '(' args ')'    {$$ = NULL;}                   
           ;

args       : argList  {$$ = $1;}
             | /* empty */         {$$ = NULL;}                       
           ;

argList    : argList ',' exp  {$$ = $1; addSibling($1, $3); }                     
             | exp      {$$ = $1;}
           ;

constant   : NUMCONST       {$$ = newExpNode(ConstantK, $1);}                         
             | CHARCONST    {$$ = newExpNode(ConstantK, $1);}                       
             | STRINGCONST  {$$ = newExpNode(ConstantK, $1);}                               
             | BOOLCONST    {$$ = newExpNode(ConstantK, $1);}                              
           ;
%%


void yyerror (const char *msg)
{ 
   cout << "Error: " <<  msg << endl;
}

int main(int argc, char **argv) {
  int option, index;
  char *file = NULL;
  extern FILE *yyin;
  syntaxTree = new TreeNode;
  //initTree();
  initTokenStrings();

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

   if(numErrors == 0){
      //printDotTree(astDot,syntaxTree, false, false);
      printTree(stdout, syntaxTree); // set to true, true for assignment 4
   }

   printf("Number of warnings: %d\n", numWarnings);
   printf("Number of errors: %d\n", numErrors);
   
   return 0;
}

