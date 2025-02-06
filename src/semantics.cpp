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
   // 10 IOLib nodes. 
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
   inputb->lineno = -1; // all are -1
   inputb->attr.name = strdup("inputb"); //We named the variables well
   inputb->type = Boolean;

   // 3
   inputc = newDeclNode(FuncK, Boolean);
   inputc->lineno = -1; // all are -1
   inputc->attr.name = strdup("inputc"); //We named the variables well
   inputc->type = Char;

   // 4
   param_output = newDeclNode(ParamK, Void);
   param_output->lineno = -1; // all are -1
   param_output->attr.name = strdup("*dummy*");
   param_output->type = Integer;

   // 5
   output = newDeclNode(FuncK, Void);
   output->lineno = -1; // all are -1
   output->attr.name = strdup("output"); //We named the variables well
   output->type = Void;
   output->child[0] = param_output;

   // 6
   param_outputb = newDeclNode(ParamK, Void);
   param_outputb->lineno = -1; // all are -1
   param_outputb->attr.name = strdup("*dummy*");
   param_outputb->type = Boolean;

   // 7
   outputb = newDeclNode(FuncK, Void);
   outputb->lineno = -1; // all are -1
   outputb->attr.name = strdup("outputb"); //We named the variables well
   outputb->type = Void;
   outputb->child[0] = param_outputb;

   // 8
   param_outputc = newDeclNode(ParamK, Void);
   param_outputc->lineno = -1; // all are -1
   param_outputc->attr.name = strdup("*dummy*");
   param_outputc->type = Char;

   // 9
   outputc = newDeclNode(FuncK, Void);
   outputc->lineno = -1; // all are -1
   outputc->attr.name = strdup("outputc"); //We named the variables well
   outputc->type = Void;
   outputc->child[0] = param_outputc;

   // 10
   outnl = newDeclNode(FuncK, Void);
   outnl->lineno = -1; // all are -1
   outnl->attr.name = strdup("outnl"); //We named the variables well
   outnl->type = Void;
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
         //insert
         //lookup
         break;
      case FuncK: 
         //insert
         //lookup
         break;
      case ParamK: 
         //insert
         //lookup
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
      case ConstantK: 
         break;
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