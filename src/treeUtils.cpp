#include "treeUtils.h"
#include <string.h>

TreeNode *newDeclNode(DeclKind kind, ExpType type, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2)
{
   TreeNode *newNode = new TreeNode;
   return newNode;
}

TreeNode *newStmtNode(StmtKind kind, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2)
{
   TreeNode *newNode = new TreeNode;
   return newNode;
}

TreeNode *newExpNode(ExpKind kind, TokenData *token, TreeNode *c0, TreeNode *c1, TreeNode *c2)
{
   TreeNode *newNode = new TreeNode;
   return newNode;
}

static void printSpaces(FILE *listing, int depth)
{
    for (int i=0; i<depth; i++) fprintf(listing, ".   ");
}

void printTreeNode(FILE *listing,
                   TreeNode *tree)
{
   fprintf(listing, "Line Number: %d" tree->lineno);
   fprintf(listing, "Line Number");
   // fprintf(listing, "Decl Node");
   // fprintf(listing, "Exp Node");
   fprintf();
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
}


