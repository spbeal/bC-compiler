#include "semantics.h"

// memory offsets are GLOBAL
static int goffset; // top of global space
static int foffset; // top of local space

/*
Compound statement
-------------------------
symtab->enter((char *)"compoundStmt");
treeTraverse(current->child[0], symtab); // process declarations
// More stuff
current->size = foffset;
treeTraverse(current->child[1], symtab);
// More stuff
symtab->leave(); //end of processing of the compound statement
//because you can throw away the scope.
*/

/*
if(isCompound) {
char *id = strdup("{");
symtab->enter("NewScope from " + (std::string)id);
}
• child[0]
• Current node
• child[1]
• child[2]
if(isCompound) {
symtab->leave();
}
• sibling
*/
TreeNode *semanticAnalysis(TreeNode *syntree, SymbolTable *symtabX, int &globalOffset)
{
   syntree = loadIOLib(syntree);
   treeTraverse(syntree, symtabX);

   globalOffset = goffset;

   return syntree;
}

/*
insert into symbol table (Any time you have a declaration)
• Saves info like the type
• Gives an error on multiple declarations of the same variable in the same scope
lookup (any time you get an id, like x=5, we need to look up x to see if we can
• put a 5 in it)
• In x=5 we look up the type of the left and right side to see if they match. (We have no
• coercion)
• If they don’t match, we have an error for that.
• X=5 the type of an assignment is the type of the left hand side of the assignment in bC
enter scope
leave scope
*/

TreeNode *loadIOLib(TreeNode *syntree) 
{
   TreeNode *input, *output, *param_output;
   TreeNode *inputb, *outputb, *param_outputb;
   TreeNode *inputc, *outputc, *param_outputc;
   TreeNode *outnl;

   // 1
   input = newDeclNode(FuncK, Integer);
   input->lineno = -1; // all are -1
   input->attr.name = strdup("input"); //We named the variables well
   input->type = Integer;

   // 2
   inputb = newDeclNode(FuncK, Boolean);

   // 3
   inputc = newDeclNode(FuncK, Boolean);
   inputc->type = Char;

   // 4
   param_output = newDeclNode(ParamK, Void);
   param_output->attr.name = strdup("*dummy*");
   param_output->type = Integer;

   // 5
   output = newDeclNode(FuncK, Void);
   output->child[0] = param_output;

   // 6
   param_outputb = newDeclNode(ParamK, Void);
   param_outputb->attr.name = strdup("*dummy*");
   param_outputb->type = Boolean;

   // 7
   outputb = newDeclNode(FuncK, Void);
   outputb->child[0] = param_outputb;

   // 8
   param_outputc = newDeclNode(ParamK, Void);
   param_outputc->attr.name = strdup("*dummy*");
   param_outputc->type = Char;

   // 9
   outputc = newDeclNode(FuncK, Void);
   outputc->child[0] = param_outputc;

   // 10
   outnl = newDeclNode(FuncK, Void);
   outnl->child[0] = NULL;

   // link them and prefix the tree we are interested in traversing.
   // This will put the symbols in the symbol table.
   input->sibling = output;
   output->sibling = inputb;
   inputb->sibling = outputb;
   outputb->sibling = inputc;
   inputc->sibling = outputc;
   outputc->sibling = outnl;
   outnl->sibling = syntree; // add in the tree we were given
   return input;
}

void decl_traverse(TreeNode * tree, SymbolTable *symtab) {

   switch (tree->kind.decl) {
      case VarK:

         break;
      case FuncK: 

         break;
      case ParamK: 

         break;
      default: break;
   }
}

void stmt_traverse(TreeNode * tree, SymbolTable *symtab) {
   switch (tree->kind.stmt) {
      case IfK:
         break;
      case WhileK: 
         break;
      case ForK: 
   
         break;
      case CompoundK: 

         break;
      case ReturnK: 
         break;
      case BreakK:
         break;
      case RangeK: 
         break;
      default: break;
   }
}

void exp_traverse(TreeNode * tree, SymbolTable *symtab) {
   switch (tree->kind.exp) {
      case AssignK: 

         break;
      case OpK: 

         break;
      case CallK: 

         break;
      case ConstantK: {
         
         break;
      }
      case IdK: 

         break;
      default: break;
   }
}

void treeTraverse(TreeNode * tree, SymbolTable *symtab)
{
   if (tree == NULL) {
      return;
   }

   switch(tree->nodekind) {
      case DeclK:
         decl_traverse(tree, symtab);
         break;
      case StmtK:
         stmt_traverse(tree, symtab);
         break;
      case ExpK:
         exp_traverse(tree, symtab);
         break;
      default: break;
   }

   if (tree->sibling) {
      treeTraverse(tree->sibling, symtab);
   }
}