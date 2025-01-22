%{
#include "scanType.h"
#include "parser.tab.h" // This has to be the last include
#include "treeNodes.h"
#include "treeUtils.h"

using namespace std;

#define YY_DECL extern "C" int yylex()
#define YYSTYPE int

int line=1;
char *lastToken=(char *)"";
//extern int numErrors;

int setValue(int linenum, int tokenClass, char *svalue) {
    yylval->tokenData->tokenclass = tokenClass;
    yylval->tokenData->linenum = linenum;
    yylval->tokenData->tokenstr = strdup(svalue);
    yylval->tokenData->cvalue = svalue[0];
    yylval->tokenData->nvalue = atoi(svalue);
    yylval->tokenData->svalue = strdup(svalue);

    switch (tokenClass) {

    case NUMCONST:
         // yylval.tinfo.nvalue = hmmmm ..... I don't need this yet, but
         break;

    case OP:
         // yylval.tinfo.cvalue = hmmm..... What should I set it to?
         break;
    }


    return tokenClass;
}

%}

%option noyywrap

letter      [a-zA-Z]
digit    [0-9]
quote    [\']
dquote [\"]
underbar [_]
letdig      {digit}|{letter}|{underbar}
limitters   [\;\,\(\)\{\}\[\]]
charcon        \'[\\]?[^'(\n)(\0)]\'
stringcon      \"([^"(\n)(\0)])*\"
op    [\-\+\*\/\%]
whitespace  [\ \t]
%%

\/\/.*         { return setValue(line, COMMENT, yytext); }
"=="           { return setValue(line, EQ, yytext); }
"!="           { return setValue(line, NEQ, yytext); }
"<="           { return setValue(line, LEQ, yytext); }
">="           { return setValue(line, GEQ, yytext); }
":<:"          { return setValue(line, MIN, yytext); }
":>:"          { return setValue(line, MAX, yytext); }
"+="           { return setValue(line, ADDASS, yytext); }
"-="           { return setValue(line, SUBASS, yytext); }
"*="           { return setValue(line, MULASS, yytext); }
"/="           { return setValue(line, DIVASS, yytext); }
"--"           { return setValue(line, DEC, yytext); }
"++"           { return setValue(line, INC, yytext); }

[\(\)]         { return setValue(line, OP, yytext); } // single char tokens

":"            { return setValue(line, OP, yytext); }
"<"            { return setValue(line, OP, yytext); }
">"            { return setValue(line, OP, yytext); }
"="            { return setValue(line, OP, yytext); }
"*"            { return setValue(line, OP, yytext); }
"-"            { return setValue(line, OP, yytext); }
"+"            { return setValue(line, OP, yytext); }
"?"            { return setValue(line, OP, yytext); }
"/"            { return setValue(line, OP, yytext); }
"%"            { return setValue(line, OP, yytext); }
"["            { return setValue(line, OP, yytext); }
"]"            { return setValue(line, OP, yytext); }
";"            { return setValue(line, OP, yytext); }
"{"            { return setValue(line, OP, yytext); }
"}"            { return setValue(line, OP, yytext); }
","            { return setValue(line, OP, yytext); }

"return"          { return setValue(line, RETURN, yytext); }
"break"          { return setValue(line, BREAK, yytext); }
"then"          { return setValue(line, THEN, yytext); }
"if"          { return setValue(line, IF, yytext); }
"for"          { return setValue(line, FOR, yytext); }
"while"          { return setValue(line, WHILE, yytext); }
"else"          { return setValue(line, ELSE, yytext); }
"static"          { return setValue(line, STATIC, yytext); }
"to"          { return setValue(line, TO, yytext); }
"do"          { return setValue(line, DO, yytext); }
"by"          { return setValue(line, BY, yytext); }



"int"          { return setValue(line, INT, yytext); }
"bool"          { return setValue(line, BOOL, yytext); }
"false"          { return setValue(line, BOOLCONST, yytext); }
"true"          { return setValue(line, BOOLCONST, yytext); }

"char"          { return setValue(line, CHAR, yytext); }
"and"          { return setValue(line, AND, yytext); }
"not"          { return setValue(line, NOT, yytext); }
"or"          { return setValue(line, OR, yytext); }

^"#DRBC ".*    { return setValue(line, PRECOMPILER, yytext); }
^"#DRBCRUN ".* { return setValue(line, PRECOMPILER, yytext); }

\n             {  line++; }

{charcon} { return setValue(line, CHARCONST, yytext); }
{stringcon}   { return setValue(line, STRINGCONST, yytext); }

{letter}{letdig}*   { return setValue(line, ID, yytext); }

[0-9]+         { return setValue(line, NUMCONST, yytext); }
{whitespace}+  ;
.              { printf("ERROR(%d): Token error, invalid or misplaced input character: \'%c\'. Character Ignored.\n", line, yytext[0]);}


