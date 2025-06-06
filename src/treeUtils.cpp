#include "treeUtils.h"
#include "parser.tab.h" // This has to be the last include
//#include "dot.h"

char * largerTokens[LASTTERM+1];
TreeNode *addSibling(TreeNode *t, TreeNode *s)
{
   //if (s == NULL) exit(0); this was my issue
   if (t == NULL) return s;

   // make sure s is not null. If it is this s a major error. Exit the program!
   // Make sure t is not null. If it is, just return s
   // look down t’s sibling list until you fin with with sibblin = null (the end o f the lsit) and add s there.
   TreeNode * curr = t;
   while (curr->sibling != NULL)
   {
      curr = curr->sibling;
   }
   curr->sibling = s;
   return t;
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
   largerTokens[STRINGCONST] = (char *)"stringconst";

   largerTokens['?'] = (char *)"?";
   largerTokens['<'] = (char *)"<";
   largerTokens['>'] = (char *)">";
   largerTokens['='] = (char *)"=";
   largerTokens['%'] = (char *)"%";
   largerTokens['-'] = (char *)"-";
   largerTokens['+'] = (char *)"+";
   largerTokens['*'] = (char *)"*";
   largerTokens['/'] = (char *)"/";

   largerTokens[LASTTERM] = (char *)"lastterm";
}

TreeNode* initializeNode(TreeNode* c0, TreeNode* c1, TreeNode* c2, TokenData *token)
{
   //static int node_count = 0;
   TreeNode* newNode = new TreeNode;
   //newNode->nodeNum = node_count++;
   newNode->child[0] = newNode->child[1] = newNode->child[2] = NULL;
   if (c0)
      newNode->child[0] = c0;
   if (c1)
      newNode->child[1] = c1;
   if (c2)
      newNode->child[2] = c2;

   newNode->sibling = NULL;

   if (token != NULL)
   {
      newNode->lineno = token->linenum; 
      newNode->attr.op = token->tokenclass;
      newNode->attr.value = token->nvalue;
      newNode->attr.cvalue = token->cvalue;
      newNode->attr.name = token->tokenstr;
      newNode->attr.string = token->svalue;
   }
   /*
    bool isStatic;                         // is staticly allocated?
    bool isArray;                          // is this an array?
    bool isConst;                          // can be computed at compile time?
    bool isUsed;                           // is this variable used?
    bool isAssigned;                       // has the variable been given a value?
    */

   newNode->isStatic = false;
   newNode->isArray = false;
   newNode->isConst = false;
   newNode->isUsed = false;
   newNode->isAssigned = false;
   //newNode->isNewLine = false;
   
   // offset
   newNode->size = 1;

   return newNode;
}

TreeNode *newDeclNode(DeclKind kind, ExpType type, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2)
{
   TreeNode *newNode = initializeNode(c0,c1,c2,token);

   newNode->nodeNum++;
   newNode->nodekind = DeclK;
   newNode->kind.decl = kind;
   newNode->type = type;

   return newNode;

   /*
   • varDeclId
   • funDecl
   • matched : FOR ID '=' iterRange DO matched
   • unmatched : FOR ID '=' iterRange DO unmatched
   */
}

TreeNode *newStmtNode(StmtKind kind, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2)
{
   // compound, matched, unmatched, iterRange, returnStmt, breakStmt
   TreeNode *newNode = initializeNode(c0,c1,c2,token);
   //int i = 0;
   newNode->nodeNum++;
   newNode->nodekind = StmtK;
   newNode->kind.stmt = kind;
   //newNode->type = type;
      //<more code>

   return newNode;
}

TreeNode *newExpNode(ExpKind kind, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2)
{
   /* exp
   • simpleExp
   • andExp
   • unaryRelExp
   • relExp
   • minmaxExpn
   • sumExp
   • mulExp
   • unaryExp
   • mutable
   • call
   • constant
   */
   TreeNode *newNode = initializeNode(c0,c1,c2,token);
   newNode->nodeNum++;
   newNode->nodekind = ExpK;
   newNode->kind.exp = kind;
   //newNode->type = Integer;
   //newNode->nodekind = kind;
   return newNode;
}

static void printSpaces(FILE *listing, int depth)
{
    for (int i=0; i<depth; i++) fprintf(listing, ".   ");
}

char *varKindToStr(int kind){
   switch(kind) {
      case None:
         return (char *)"None";
      case Local:
         return (char *)"Local";
      case Global:
         return (char *)"Global";
      case Parameter:
         return (char *)"Parameter";
      case LocalStatic:
         return (char *)"LocalStatic";
      default:
         return (char *)"unknownVarKind";
   }
}

char * variable_kind_str(TreeNode * tree)
{
   static char return_str[50];
   return_str[0] = '\0';

   switch (tree->varKind)
   {
      // None, Local, Parameter, Global, LocalStatic
      case None: strcat(return_str, "None");
         break;

      case Local: strcat(return_str, "Local");
         break;

      case Global: strcat(return_str, "Global");
         break;

      case Parameter: strcat(return_str, "Parameter");
         break;

      case LocalStatic: strcat(return_str, "LocalStatic");
         break;

      default: strcat(return_str, "unknownVarKind");
         break;
   }
   return return_str;
}


void showAllocation(FILE * listing, TreeNode * tree)
{
   fprintf(listing, " [mem: %s loc: %d size: %d]", varKindToStr(tree->varKind), tree->offset, tree->size);
}

// char* type_str(ExpType type, bool isStatic, bool isArray) {
//    //char * return_str = "";
//    static char return_str[50];
//    return_str[0] = '\0';
//    if (isStatic) strcat(return_str, "static ");
//    if (isArray) strcat(return_str, "array of ");

//    switch(type) {
//       case Integer: strcat(return_str, "type int"); break;
//       case Boolean: strcat(return_str, "type bool"); break;
//       case Char:    strcat(return_str, "type char"); break;
//       case Void:    strcat(return_str, "type void"); break;
//       case UndefinedType: strcat(return_str, "type UndefinedType"); break;
//       default:      strcat(return_str, "invalid"); break;
//    }

//    return return_str;
// }

char* type_str(ExpType type, bool isStatic, bool isArray) {
   // Use a static array of buffers so each call gets a different one
   static char buffers[4][50]; // Supports up to 4 nested/parallel calls
   static int next = 0;

   char *return_str = buffers[next];
   next = (next + 1) % 4;

   return_str[0] = '\0';
   if (isStatic) strcat(return_str, "static ");
   if (isArray) strcat(return_str, "array of ");

   switch(type) {
      case Integer: strcat(return_str, "type int"); break;
      case Boolean: strcat(return_str, "type bool"); break;
      case Char:    strcat(return_str, "type char"); break;
      case Void:    strcat(return_str, "type void"); break;
      case UndefinedType: strcat(return_str, "type UndefinedType"); break;
      default:      strcat(return_str, "invalid"); break;
   }

   return return_str;
}


void printTreeNode(FILE *listing, TreeNode *tree)
{
   char * print_str;
   print_str = type_str(tree->type, tree->isStatic, tree->isArray);
   switch (tree->nodekind)
   {      
      case DeclK:
         switch (tree->kind.decl) {
            case VarK:
               fprintf(listing, "Var: %s of %s", 
                  tree->attr.name, print_str); 
               showAllocation(listing, tree);
               break;
            case FuncK: 
               fprintf(listing, "Func: %s returns %s", 
                  tree->attr.name, print_str); 
               showAllocation(listing, tree);
               break;
            case ParamK: 
               // check for arrays
               fprintf(listing, "Parm: %s of %s", 
                  tree->attr.name,print_str); 
               showAllocation(listing, tree);
               break;
            default: fprintf(listing, "invalid"); 
               break;
         }
         break;
      case StmtK:
         switch (tree->kind.stmt) {
            case IfK:
               fprintf(listing, "If"); 
               break;
            case WhileK: 
               fprintf(listing, "While"); 
               break;
            case ForK: 
               // fix var: i of type int
               // range as a child
               fprintf(listing, "For");   
               showAllocation(listing, tree);             
               break;
            case CompoundK: 
               fprintf(listing, "Compound"); 
               showAllocation(listing, tree);
               break;
            case ReturnK: 
               fprintf(listing, "Return"); 
               break;
            case BreakK:
               fprintf(listing, "Break"); 
               break;
            case RangeK: 
               fprintf(listing, "Range"); 
               break;
            default: fprintf(listing, "invalid"); 
               break;
         }
         break;
      case ExpK:
         switch (tree->kind.exp) {
            case AssignK: 
               fprintf(listing, "Assign: %s of %s", 
                  tree->attr.name, print_str); 
               break;
            case OpK: 
               fprintf(listing, "Op: %s of %s", 
                  tree->attr.name, print_str); 
                  // tree->isStatic, tree->isArray
               break;
            case CallK: 
               fprintf(listing, "Call: %s of %s", 
                  tree->attr.name, print_str); 
               break;
            case ConstantK: {
               if (tree->type == Char)
               {
                  //fprintf(listing, "this is it precisely:\"%s\"", tree->attr.name);
                  // newline case
                  char str[5];
                  str[0] = '\'';
                  str[1] = '\\';
                  str[2] = 'n';
                  str[3] = '\'';
                  str[4] = '\0';
                  if (tree->isArray) {
                     fprintf(listing, "Const %s of %s", tree->attr.name, print_str);
                     showAllocation(listing, tree);
                  }
                  // else if (strcmp(str, tree->attr.name) == 0) 
                  // {
                  //    fprintf(listing, "Const '%c' of %s", '\n', print_str);  
                  // }
                  else {
                     fprintf(listing, "Const %s of %s", tree->attr.name, print_str);
                  }
               } 
               else
               {
                  fprintf(listing, "Const %s of %s", tree->attr.name, print_str); 
               }
               // }
               // if (tree->attr.name == ) {
               // //if (tree->attr.name == str) {
               //    //fprintf(listing, "Const ");
               //    //fprintf(listing, "\n");
               //    //fprintf(listing, "'");
               //    fprintf(listing, "Const '%c'", '\n');
               //    break;
               // }
               // fprintf(listing, "Const %s", tree->attr.name); 
               break;
            }
            case IdK: 
               fprintf(listing, "Id: %s of %s", 
                  tree->attr.name, print_str); 
               showAllocation(listing, tree);
               break;
            default: fprintf(listing, "invalid"); 
               break;
         }
         break;

      default:
         fprintf(listing, "Invalid nodekind");
   }

   fprintf(listing, " [line: %d]", tree->lineno);
}


void printTreeRec(FILE *listing, int depth, int siblingCnt, TreeNode *tree)
{
   int childCnt;
   if(tree == NULL) return;
   if (tree!=NULL) {
      // print self
      printTreeNode(listing, tree);
      fprintf(listing, "\n");

      // print children
      for (childCnt = 0; childCnt<MAXCHILDREN; childCnt++) {
         if (tree->child[childCnt]) {
            printSpaces(listing, depth);
            fprintf(listing, "Child: %d  ", childCnt);
            printTreeRec(listing, depth+1, 1, tree->child[childCnt]);
         }
      }
   }
   // print sibling
   tree = tree->sibling;
   if (tree!=NULL) {
      if (depth) {
         printSpaces(listing, depth-1);
         fprintf(listing, "Sibling: %d  ", siblingCnt);
      }
      printTreeRec(listing, depth, siblingCnt+1, tree);
   }
   fflush(listing);
}

void printTree(FILE *listing, TreeNode *tree)
{
   if(tree == NULL){
      printf("NULL tree\n");
      return;
   }
   printTreeRec(listing, 1, 1, tree);
   //printDotTree(astDot, syntaxTree, false, false);
}

