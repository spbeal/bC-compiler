%{
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "scanType.h"
#include "treeNodes.h"
#include "treeUtils.h"



using namespace std;

extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;

void yyerror(const char *msg);

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

// the syntax tree goes here
int numErrors;
int numWarnings;
extern int line;

TreeNode *addSibling(TreeNode *t, TreeNode *s)
{
   if (s == NULL) exit(0);
   if (t == NULL) return s;
   // make sure s is not null. If it is this s a major error. Exit the program!
   // Make sure t is not null. If it is, just return s
   // look down t’s sibling list until you fin with with sibblin = null (the end o f the lsit) and add s there.
   TreeNode * curr = t;
   while (curr->sibling != NULL)
   {
      curr = t->sibling;
   }
   curr->sibling = s;
   return s;
}
// pass the static and type attribute down the sibling list
void setType(TreeNode *t, ExpType type, bool isStatic)
{
  TreeNode * curr = t;
   while (curr != NULL) {
      curr->type = type;
      curr->isStatic = isStatic;
      curr = curr->sibling;
   // set t->type and t->isStatic
   // t = t->sibling;
   }
}
TreeNode *syntaxTree;

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
%token <tokenData> '(' ')' ',' ';' '[' '{' '}' ']' ':'
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

%token <tokenData> LASTTERM

%%
program : precomList declList {syntaxTree = $2;}
    ;
precomList : precomList PRECOMPILER {$$ = NULL;}
    | PRECOMPILER                   {$$=NULL; printf("%s\n", yylval.tokenData->tokenstr);}
    | /* empty */                   {$$=NULL;}
    ;
declList : declList decl {$$ = addSibling($1, $2);}
    | decl      {$$ = $1;}
    ;
decl : varDecl  {$$=$1;}
    | funDecl   {$$=$1;}
    ;
varDecl : typeSpec varDeclList ';' {$$ = $2; setType($2, $1, false); yyerrok;}
    ;
scopedVarDecl : STATIC typeSpec varDeclList ';' {$$ = $3; setType($3, $2, true); yyerrok;}
    | typeSpec varDeclList ';' {$$ = $2; setType($2, $1, false); yyerrok;}
    ;
varDeclList : varDeclList ',' varDeclInit {$$ = addSibling($1, $3);}
    | varDeclInit {$$ = $1;}
    ;
varDeclInit : varDeclId { $$ = $1;}
    | varDeclId ':' simpleExp {$$ = $1; if ($$ != NULL) $$->child[0] = $3;}
    ;
varDeclId : ID {newDeclNode(VarK, UndefinedType, $1);}
    | ID '[' NUMCONST ']' {$$->isArray = true; $$ = newDeclNode(VarK, Integer, $1);}
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
    | ID '[' ']' {$$->isArray = true; $$ = newDeclNode(ParamK, UndefinedType, $1);}
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
iterRange : simpleExp TO simpleExp {$$ = newStmtNode(RangeK, $1, $3);}
    | simpleExp TO simpleExp BY simpleExp {$$ = newStmtNode(RangeK, $1, $3, $5);}
    ;
unmatched  : IF simpleExp THEN stmt             {$$ = newStmtNode(IfK, $1, $2, $4);}        
             | IF simpleExp THEN matched ELSE unmatched {$$ = newStmtNode(IfK, $1, $2, $4, $6);} 
             | WHILE simpleExp DO unmatched     {$$ = newStmtNode(WhileK, $1, $2, $4);}          
             | FOR ID '=' iterRange DO unmatched {$$ = newStmtNode(ForK, $1, NULL, $4, $6); newDeclNode(VarK, Integer, $2);}      
           ;
expStmt    : exp ';'  {$$ = 1;}
             | ';'    {$$ = NULL;}                                   
           ;
compoundStmt : '{' localDecls stmtList '}'      {$$ = newStmtNode(CompoundK, $1, $2, $3); yyerrok;}
    ;
localDecls : localDecls scopedVarDecl    {;}         
             | /* empty */      {;}                        
             ;
stmtList : stmtList stmt    {$$ = ($2==NULL ? $1 : addSibling($1, $2)); }
    |           {;}
    ;
returnStmt : RETURN ';'      {$$ = newStmtNode(ReturnK, $1);}                           
             | RETURN exp ';'    {$$ = newStmtNode(ReturnK, $1, $2);}                            
           ;
breakStmt  : BREAK ';'               {$$ = newStmtNode(BreakK, $1);}           
           ;

exp        : mutable assignop exp     {$$ = newExpNode(AssignK, $2, $1, $3);}                  
             | mutable INC            {$$ = newExpNode(AssignK, $2, $1);}                 
             | mutable DEC            {$$ = newExpNode(AssignK, $2, $1);}              
             | simpleExp              {$$ = $1;}
           ;

assignop  : '=' {;}
            | ADDASS {;}
            | SUBASS {;}
            | MULASS {;}
            | DIVASS {;}
          ;

simpleExp  : simpleExp OR andExp {$$ = newExpNode(Opk, $2, $1, $3);}            
             | andExp {$$ = $1;}
           ;

andExp     : andExp AND unaryRelExp  {$$ = newExpNode(Opk, $2, $1, $3);}                      
             | unaryRelExp          {$$ = $1;}
           ;

unaryRelExp : NOT unaryRelExp     {$$ = newExpNode(Opk, $1, $2);}                        
              | relExp              {$$ = $1;}
            ;

relExp     : minmaxExp relop minmaxExp    {$$ = newExpNode(Opk, $2, $1, $3);}            
             | minmaxExp                    {$$ = $1;}
           ;

relop      : LEQ {;}
             | '<' {;}
             | '>' {;}
             | GEQ {;}
             | EQ {;}
             | NEQ {;}
           ;

minmaxExp  : minmaxExp minmaxop sumExp  {;}            
             | sumExp  {;}
           ;

minmaxop   : MAX {;}
             | MIN {;}
           ;

sumExp     : sumExp sumop mulExp  {;}            
             | mulExp {;}
           ;

sumop      : '+' {;}
             | '-' {;}
           ;

mulExp     : mulExp mulop unaryExp  {;}         
             | unaryExp  {;}
           ;

mulop      : '*' {;}
             | '/' {;}
             | '%' {;}
           ;

unaryExp   : unaryop unaryExp  {;}                 
             | factor   {;}
           ;

unaryop    : '-'        {;}                                     
             | '*'  {;}                                      
             | '?'  {;}
             ;
           ;

factor     : immutable {;}
             | mutable {;}
           ;

mutable    : ID    {$$ = newExpNode(IdK, $1);}                                   
             | ID '[' exp ']'   {$$ = newExpNode(IdK, $1);}                       
           ;

immutable  : '(' exp ')'    {;}                        
             | call         {;}
             | constant     {;}
           ;

call       : ID '(' args ')'    {;}                   
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

char * largerTokens[LASTTERM+1];
void initTokenStrings()
{ 
   for (int x = 0; x < LASTTERM+1; x++)
   {
      largerTokens[x] = (char *)"Undefined largerToken";
   }
   largerTokens[ADDASS] = (char *)"+=";
   largerTokens[AND] = (char *)"and";
   largerTokens[BOOL] = (char *)"bool";
   largerTokens[BOOLCONST] = (char *)"boolconst";
   largerTokens[BREAK] = (char *)"break";
   largerTokens[BY] = (char *)"by";
   largerTokens[CHAR] = (char *)"char";
   largerTokens[CHARCONST] = (char *)"charconst";
   largerTokens[CHSIGN] = (char *)"chsign";
   largerTokens[DEC] = (char *)"--";
   largerTokens[DIVASS] = (char *)"/=";
   largerTokens[DO] = (char *)"do";
   largerTokens[ELSE] = (char *)"else";
   largerTokens[EQ] = (char *)"==";
   largerTokens[FOR] = (char *)"for";
   largerTokens[GEQ] = (char *)">=";
   largerTokens[ID] = (char *)"id";
   largerTokens[IF] = (char *)"if";
   largerTokens[INC] = (char *)"++";
   largerTokens[INT] = (char *)"int";
   largerTokens[LEQ] = (char *)"<=";
   largerTokens[MAX] = (char *)":>:";
   largerTokens[MIN] = (char *)":<:";
   largerTokens[MULASS] = (char *)"*=";
   largerTokens[NEQ] = (char *)"!=";
   largerTokens[NOT] = (char *)"not";
   largerTokens[NUMCONST] = (char *)"numconst";
   largerTokens[OR] = (char *)"or";
   largerTokens[RETURN] = (char *)"return";
   largerTokens[SIZEOF] = (char *)"sizeof";
   largerTokens[STATIC] = (char *)"static";
   largerTokens[STRINGCONST] = (char *)"stringconst";
   largerTokens[SUBASS] = (char *)"-=";
   largerTokens[THEN] = (char *)"then";
   largerTokens[TO] = (char *)"to";
   largerTokens[WHILE] = (char *)"while";
   largerTokens[LASTTERM] = (char *)"lastterm";
}

void yyerror (const char *msg)
{ 
   cout << "Error: " <<  msg << endl;
}

int main(int argc, char **argv) {
   int option, index;
   char *file = NULL;
   extern FILE *yyin;
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
      initTokenStrings();
      printTree(stdout, syntaxTree); // set to true, true for assignment 4
   }
   return 0;
}

