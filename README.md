# bC-compiler
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test00.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test00.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test0.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test0.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test0multilinePrecompiler.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test0multilinePrecompiler.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test0setValue.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test0setValue.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test1.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test1.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test2.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test2.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test3.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test3.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test4.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test4.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test5.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test5.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test6.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test6.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test7SlashN.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test7SlashN.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test8setType.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/test8setType.bC)
diff -y <(./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/tictactoe.bC) <(/y/shared/Engineering/cs-drbc/cs445/bC 3  /y/shared/Engineering/cs-drbc/cs445/bC_in_3/tictactoe.bC)

./bC /y/shared/Engineering/cs-drbc/cs445/bC_in_3/


program : precomList declList {syntaxTree = $2;}
    ;
precomList : precomList PRECOMPILER {$$= $1;}
    | PRECOMPILER                   {$$=NULL; printf("%s\n", yylval.tokenData->tokenstr);}
    | /* empty */                   {$$=NULL;}
    ;
declList : declList decl {$$ = addSibling( $1, $2);}
    | decl      {$$ = $1;}
    ;
decl : varDecl  {$$=$1;}
    | funDecl   {$$=$1;}
    ;
varDecl : typeSpec varDeclList ';' {$$ = $2; setType ($2, $1, false); yyerrok;}
    ;
scopedVarDecl : STATIC typeSpec varDeclList ';' {;}
    | typeSpec varDeclList ';' {;}
    ;
varDeclList : varDeclList ',' varDeclInit {;}
    | varDeclInit {;}
    ;
varDeclInit : varDeclId {;}
    | varDeclId ':' simpleExp {$$ = $1; if ($$ != NULL) $$->child[0] = $3;}
    ;
varDeclId : ID {;}
    | ID '[' NUMCONST ']' {;}
    ;
typeSpec : INT {;}
    | BOOL {;}
    | CHAR {;}
    ;
funDecl : typeSpec ID '(' parms ')' stmt {$$ = newDeclNode(FuncK, $1, $2, $4, $6);}
    | ID '(' parms ')' stmt {$$ = newDeclNode(FuncK, Void, $1, $3, $5);}
    ;
parms : parmList {;}
    ;
parmList : parmList ';' parmTypeList {;}
    | parmTypeList {;}
    ;
parmTypeList : typeSpec parmIdList {;}
    ;
parmIdList: parmIdList ',' parmId {;}
    | parmId {;}
    ;
parmId: ID {;}
    | ID '[' ']' {;}
    ;
stmt : matched {$$ = $1;}
    | unmatched {$$ = $1;}
    ;
matched : IF simpleExp THEN matched ELSE matched { $$ = newStmtNode(IfK, $1, $2, $4, $6);}
    | WHILE simpleExp DO matched { $$ = newStmtNode(WhileK, $1, $2, $4);}
    | FOR ID '=' iterRange DO matched { $$ = newStmtNode(ForK, $1, NULL, $4, $6);}
    | expStmt {;}
    | compoundStmt {;}
    | returnStmt {;}
    | breakStmt {;}
    ;
iterRange : simpleExp TO simpleExp {;}
    | simpleExp TO simpleExp BY simpleExp {;}
    ;
unmatched  : IF simpleExp THEN stmt             {$$ = newStmtNode(IfK, $1, $2, $4);}
             | IF simpleExp THEN matched ELSE unmatched {$$ = newStmtNode(IfK, $1, $2, $4, $6);}
             | WHILE simpleExp DO unmatched     {$$ = newStmtNode(WhileK, $1, $2, $4);}
             | FOR ID '=' iterRange DO unmatched {$$ = newStmtNode(ForK, $1, NULL, $4, $6); newDeclNode(VarK, Integer, $2);}
           ;
expStmt    : exp ';'  {;}
             | ';'    {;}                                     {/* NULL*/}
           ;
compoundStmt : '{' localDecls stmtList '}'      {$$ = newStmtNode(CompoundK, $1, $2, $3); yyerrok;}
    ;
localDecls : localDecls scopedVarDecl    {;}          {/* addSibling*/}
             | /* empty */      {;}                         { /* NULL*/}
             ;
stmtList : stmtList stmt    {$$ = ($2==NULL ? $1 : addSibling($1, $2)); }
    |           {;}
    ;
returnStmt : RETURN ';'      {;}                             {/* newStmtNode*/}
             | RETURN exp ';'    {;}                          {/* newStmtNode*/}
           ;
breakStmt  : BREAK ';'              {;}                   {/* newStmtNode*/}
           ;

exp        : mutable assignop exp     {;}           {/* newExpNode*/}
             | mutable INC            {;}                  {/* newExpNode*/}
             | mutable DEC            {;}                  {/* newExpNode*/}
             | simpleExp              {;}
           ;

assignop  : '=' {;}
            | ADDASS {;}
            | SUBASS {;}
            | MULASS {;}
            | DIVASS {;}
          ;

simpleExp  : simpleExp OR andExp {;}                {/* newExpNode*/}
             | andExp {;}
           ;

andExp     : andExp AND unaryRelExp  {;}            {/* newExpNode*/}
             | unaryRelExp          {;}
           ;

unaryRelExp : NOT unaryRelExp     {;}                 {/* newExpNode*/}
              | relExp              {;}
            ;

relExp     : minmaxExp relop minmaxExp    {;}      {/* newExpNode*/}
             | minmaxExp                    {;}
           ;

relop      : LEQ {;}
             | '<' {;}
             | '>' {;}
             | GEQ {;}
             | EQ {;}
             | NEQ {;}
           ;

minmaxExp  : minmaxExp minmaxop sumExp  {;}              {/* newExpNode*/}
             | sumExp  {;}
           ;

minmaxop   : MAX {;}
             | MIN {;}
           ;

sumExp     : sumExp sumop mulExp  {;}              {/* newExpNode*/}
             | mulExp {;}
           ;

sumop      : '+' {;}
             | '-' {;}
           ;

mulExp     : mulExp mulop unaryExp  {;}           {/* newExpNode*/}
             | unaryExp  {;}
           ;

mulop      : '*' {;}
             | '/' {;}
             | '%' {;}
           ;

unaryExp   : unaryop unaryExp  {;}                 {/* newExpNode*/}
             | factor   {;}
           ;

unaryop    : '-'        {;}                                     {/*$1->tokenclass=CHSIGN; $$=$1;*/}
             | '*'  {;}                                      {/*$1->tokenclass=SIZEOF; $$=$1;*/}
             | '?'  {;}
             ;
           ;

factor     : immutable {;}
             | mutable {;}
           ;

mutable    : ID    {$$ = newExpNode(IdK, $1);}
             | ID '[' exp ']'   {$$ = $1;}                       {/* newExpNode*/}
           ;

immutable  : '(' exp ')'    {;}                        {/* DRBC Note: Be careful!*/}
             | call         {;}
             | constant     {;}
           ;

call       : ID '(' args ')'    {;}                    {/*newExpNode*/}
           ;

args       : argList  {;}
             | /* empty */         {;}                       { /* NULL;*/}
           ;

argList    : argList ',' exp  {;}                     {/* addSibling*/}
             | exp      {;}
           ;

constant   : NUMCONST       {;}                            {/* newExpNode:  also set type and attr.string*/}
             | CHARCONST    {;}                               {/* newExpNode;  also set type and attr.string*/}
             | STRINGCONST  {;}                               {/* newExpNode; $$->size = $1->nvalue + 1; $$->isArray = true; also set type and attr.string*/}
             | BOOLCONST    {;}                               {/* newExpNode;  also set type and attr.string*/}
           ;