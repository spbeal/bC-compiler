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

int numErrors;
int numWarnings;
extern int line;
extern int yylex();

TreeNode *addSibling(TreeNode *t, TreeNode *s)
{
   if (s == NULL) exit(0);
   if (t == NULL) return s;
   // make sure s is not null. If it is this s a major error. Exit the program!
   // Make sure t is not null. If it is, just return s
   // look down t’s sibling list until you fin with with sibblin = null (the end o f the lsit) and add s there.
   while (t->sibling != NULL)
   {
      t = t->sibling;
   }
   t->sibling = s;
   return s;
}
// pass the static and type attribute down the sibling list
void setType(TreeNode *t, ExpType type, bool isStatic)
{
   while (t) {
      t->type = type;
      t->isStatic = isStatic;
      t = t->sibling;
   // set t->type and t->isStatic
   // t = t->sibling;
   }
}
// the syntax tree goes here
TreeNode *syntaxTree;

%}
%union
{
   TokenData *tokenData;
   TreeNode *tree;
   ExpType type; // for passing type spec up the tree
   // struct   TokenData tokenData ;
}

// %type  <tree>  OP
// %type  <tree>  NEQ
// %type  <tree>  AND
// %type  <tree>  NOT
// %type  <tree>  OR
// %type  <tree>  PRECOMPILER
// %type  <tree>  NUMCONST

// %type  <tree>  EQ
// %type  <tree>  LEQ
// %type  <tree>  GEQ
// %type  <tree>  MAX
// %type  <tree>  MIN
// %type  <tree>  ADDASS
// %type  <tree>  SUBASS
// %type  <tree>  MULASS
// %type  <tree>  DIVASS
// %type  <tree>  DEC
// %type  <tree>  INC

// %type  <tree>  INT
// %type  <tree>  BOOL
// %type  <tree>  BOOLCONST
// %type  <tree>  CHAR

// %type  <tree>  ID
// %type  <tree>  RETURN
// %type  <tree>  BREAK
// %type  <tree>  THEN
// %type  <tree>  IF
// %type  <tree>  FOR
// %type  <tree>  WHILE
// %type  <tree>  ELSE
// %type  <tree>  STATIC
// %type  <tree>  TO
// %type  <tree>  DO
// %type  <tree>  BY

// %type  <tree>  COMMENT

// %type  <tree>  CHARCONST
// %type  <tree>  STRINGCONST
// %type   <tree>  ERROR 

%token <tokenData> FIRSTOP
%token <tokenData> LASTOP
%token <tokenData> LASTTERM
%type <tree> term program
%type <type> typeSpec

%token <tokenData> '(' ')' ',' ';' '[' '{' '}' ']' ':'

%token   <tokenData>  OP
%token   <tokenData>  NEQ
%token   <tokenData>  AND
%token   <tokenData>  NOT
%token   <tokenData>  OR
%token   <tokenData>  PRECOMPILER
%token   <tokenData>  NUMCONST

%token   <tokenData>  EQ
%token   <tokenData>  LEQ
%token   <tokenData>  GEQ
%token   <tokenData>  MAX
%token   <tokenData>  MIN
%token   <tokenData>  ADDASS
%token   <tokenData>  SUBASS
%token   <tokenData>  MULASS
%token   <tokenData>  DIVASS
%token   <tokenData>  DEC
%token   <tokenData>  INC
%token   <tokenData>  SIZEOF


%token   <tokenData>  INT
%token   <tokenData>  BOOL
%token   <tokenData>  BOOLCONST
%token   <tokenData>  CHAR

%token   <tokenData>  ID
%token   <tokenData>  RETURN
%token   <tokenData>  BREAK
%token   <tokenData>  THEN
%token   <tokenData>  IF
%token   <tokenData>  FOR
%token   <tokenData>  WHILE
%token   <tokenData>  ELSE
%token   <tokenData>  STATIC
%token   <tokenData>  TO
%token   <tokenData>  DO
%token   <tokenData>  BY

%token   <tokenData>  COMMENT

%token   <tokenData>  CHARCONST
%token   <tokenData>  CHSIGN
%token   <tokenData>  STRINGCONST

%token   <tokenData>  ERROR 
// %type <tokenData>  term program
%%
program  :  program term
   |  term  {$$=$1;}
   ;
term  : 
      OP {printToken(yylval->tokenData, "OP");}
   |  EQ {printToken(yylval->tokenData, "EQ");}
   |  NEQ {printToken(yylval->tokenData, "NEQ");}
   |  LEQ {printToken(yylval->tokenData, "LEQ");}
   |  GEQ {printToken(yylval->tokenData, "GEQ");}
   |  MAX {printToken(yylval->tokenData, "MAX");}
   |  MIN {printToken(yylval->tokenData, "MIN");}
   |  ADDASS {printToken(yylval->tokenData, "ADDASS");}
   |  MULASS {printToken(yylval->tokenData, "MULASS");}
   |  SUBASS {printToken(yylval->tokenData, "SUBASS");}
   |  DIVASS {printToken(yylval->tokenData, "DIVASS");}
   |  DEC {printToken(yylval->tokenData, "DEC");}
   |  INC {printToken(yylval->tokenData, "INC");}
   |  AND {printToken(yylval->tokenData, "AND");}
   |  NOT {printToken(yylval->tokenData, "NOT");}
   |  OR {printToken(yylval->tokenData, "OR");}

   |  COMMENT {;}

   |  INT {printToken(yylval->tokenData, "INT");}
   |  BOOL {printToken(yylval->tokenData, "BOOL");}
   |  BOOLCONST {printToken(yylval->tokenData, "BOOLCONST");}
   |  CHAR {printToken(yylval->tokenData, "CHAR");}

   |  RETURN {printToken(yylval->tokenData, "RETURN");}
   |  BREAK {printToken(yylval->tokenData, "BREAK");}
   |  THEN {printToken(yylval->tokenData, "THEN");}
   |  IF {printToken(yylval->tokenData, "IF");}
   |  FOR {printToken(yylval->tokenData, "FOR");}
   |  WHILE {printToken(yylval->tokenData, "WHILE");}
   |  ELSE {printToken(yylval->tokenData, "ELSE");}
   |  STATIC {printToken(yylval->tokenData, "STATIC");}
   |  TO {printToken(yylval->tokenData, "TO");}
   |  DO {printToken(yylval->tokenData, "DO");}
   |  BY {printToken(yylval->tokenData, "BY");}


   |  ID {printToken(yylval->tokenData, "ID");}
   |  CHARCONST {printToken(yylval->tokenData, "CHARCONST");}
   |  STRINGCONST {printToken(yylval->tokenData, "STRINGCONST");}
   |  PRECOMPILER {printToken(yylval->tokenData, "PRECOMPILER");}
   |  NUMCONST {printToken(yylval->tokenData, "NUMCONST");}
   |  ERROR    {cout << "ERROR(SCANNER Line " << yylval->tokenData.linenum << "): Invalid input character " << yylval->tokenData.tokenstr << endl; }
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
   return 0;
}

