#include "treeUtils.h"
#include <string.h>
#include "parser.tab.h" // This has to be the last include
//#include "dot.h"


char * largerTokens[LASTTERM+1];
TreeNode *addSibling(TreeNode *t, TreeNode *s)
{
   if (s == NULL) return s;
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

TreeNode* initalizeNode(TreeNode* c0, TreeNode* c1, TreeNode* c2, TokenData *token)
{
   TreeNode* newNode = new TreeNode;

   newNode->child[0] = c0;
   newNode->child[1] = c1;
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
   // offset
   newNode->size = 1;

   return newNode;
}

TreeNode *newDeclNode(DeclKind kind, ExpType type, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2)
{
   TreeNode *newNode = initalizeNode(c0,c1,c2,token);

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
   TreeNode *newNode = initalizeNode(c0,c1,c2,token);
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
   TreeNode *newNode = initalizeNode(c0,c1,c2,token);
   newNode->nodeNum++;
   newNode->nodekind = ExpK;
   newNode->kind.exp = kind;
   //newNode->type = type;
   //newNode->nodekind = kind;
   return newNode;
}

static void printSpaces(FILE *listing, int depth)
{
    for (int i=0; i<depth; i++) fprintf(listing, ".   ");
}

void printTreeNode(FILE *listing,
                   TreeNode *tree, bool a, bool b)
{

   switch (tree->nodekind)
   {
      case DeclK:
         switch(tree->kind.decl)
         {
            default:
               fprintf(listing, "Invalid kind");
               break;
         }

         break;

      case StmtK:
         switch(tree->kind.stmt)
         {
            default:
               fprintf(listing, "Invalid kind");
               break;
         }

         break;

      case ExpK:
         switch(tree->kind.exp)
         {
            default:
               fprintf(listing, "Invalid kind");
               break;
         }

         break;

      default:
         fprintf(listing, "Invalid nodekind");
   }

   fprintf(listing, "[line: %d]", tree->lineno);
   // fprintf(listing, "Decl Node");
   // fprintf(listing, "Exp Node");

   /*
      newNode->child[0] = c0;
   newNode->child[1] = c1;
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
   */
}


void printTreeRec(FILE *listing, int depth, int siblingCnt, TreeNode *tree)
{
   int childCnt;
   if(tree == NULL) return;
   if (tree!=NULL) {
      // print self
      printTreeNode(listing, tree, true, true);
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


