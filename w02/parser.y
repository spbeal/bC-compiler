%{
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include "scanType.h"
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

%}
%union
{
   struct   TokenData tinfo ;
}
%token   <tinfo>  OP
%token   <tinfo>  NEQ
%token   <tinfo>  AND
%token   <tinfo>  NOT
%token   <tinfo>  OR
%token   <tinfo>  PRECOMPILER
%token   <tinfo>  NUMCONST

%token   <tinfo>  EQ
%token   <tinfo>  LEQ
%token   <tinfo>  GEQ
%token   <tinfo>  MAX
%token   <tinfo>  MIN
%token   <tinfo>  ADDASS
%token   <tinfo>  SUBASS
%token   <tinfo>  MULASS
%token   <tinfo>  DIVASS
%token   <tinfo>  DEC
%token   <tinfo>  INC

%token   <tinfo>  INT
%token   <tinfo>  BOOL
%token   <tinfo>  BOOLCONST
%token   <tinfo>  CHAR

%token   <tinfo>  ID
%token   <tinfo>  RETURN
%token   <tinfo>  BREAK
%token   <tinfo>  THEN
%token   <tinfo>  IF
%token   <tinfo>  FOR
%token   <tinfo>  WHILE
%token   <tinfo>  ELSE
%token   <tinfo>  STATIC
%token   <tinfo>  TO
%token   <tinfo>  DO
%token   <tinfo>  BY

%token   <tinfo>  COMMENT

%token   <tinfo>  CHARCONST
%token   <tinfo>  STRINGCONST


%token   <tinfo>  ERROR 
%type <tinfo>  term program
%%
program  :  program term
   |  term  {$$=$1;}
   ;
term  : 
      OP {printToken(yylval.tinfo, "OP");}
   |  EQ {printToken(yylval.tinfo, "EQ");}
   |  NEQ {printToken(yylval.tinfo, "NEQ");}
   |  LEQ {printToken(yylval.tinfo, "LEQ");}
   |  GEQ {printToken(yylval.tinfo, "GEQ");}
   |  MAX {printToken(yylval.tinfo, "MAX");}
   |  MIN {printToken(yylval.tinfo, "MIN");}
   |  ADDASS {printToken(yylval.tinfo, "ADDASS");}
   |  MULASS {printToken(yylval.tinfo, "MULASS");}
   |  SUBASS {printToken(yylval.tinfo, "SUBASS");}
   |  DIVASS {printToken(yylval.tinfo, "DIVASS");}
   |  DEC {printToken(yylval.tinfo, "DEC");}
   |  INC {printToken(yylval.tinfo, "INC");}
   |  AND {printToken(yylval.tinfo, "AND");}
   |  NOT {printToken(yylval.tinfo, "NOT");}
   |  OR {printToken(yylval.tinfo, "OR");}

   |  COMMENT {;}

   |  INT {printToken(yylval.tinfo, "INT");}
   |  BOOL {printToken(yylval.tinfo, "BOOL");}
   |  BOOLCONST {printToken(yylval.tinfo, "BOOLCONST");}
   |  CHAR {printToken(yylval.tinfo, "CHAR");}

   |  RETURN {printToken(yylval.tinfo, "RETURN");}
   |  BREAK {printToken(yylval.tinfo, "BREAK");}
   |  THEN {printToken(yylval.tinfo, "THEN");}
   |  IF {printToken(yylval.tinfo, "IF");}
   |  FOR {printToken(yylval.tinfo, "FOR");}
   |  WHILE {printToken(yylval.tinfo, "WHILE");}
   |  ELSE {printToken(yylval.tinfo, "ELSE");}
   |  STATIC {printToken(yylval.tinfo, "STATIC");}
   |  TO {printToken(yylval.tinfo, "TO");}
   |  DO {printToken(yylval.tinfo, "DO");}
   |  BY {printToken(yylval.tinfo, "BY");}


   |  ID {printToken(yylval.tinfo, "ID");}
   |  CHARCONST {printToken(yylval.tinfo, "CHARCONST");}
   |  STRINGCONST {printToken(yylval.tinfo, "STRINGCONST");}
   |  PRECOMPILER {printToken(yylval.tinfo, "PRECOMPILER");}
   |  NUMCONST {printToken(yylval.tinfo, "NUMCONST");}
   |  ERROR    {cout << "ERROR(SCANNER Line " << yylval.tinfo.linenum << "): Invalid input character " << yylval.tinfo.tokenstr << endl; }
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

