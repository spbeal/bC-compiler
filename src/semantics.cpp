#include "semantics.h"


TreeNode *semanticAnalysis(TreeNode *syntree, SymbolTable *symtabX, int &globalOffset)
{
   syntree = loadIOLib(syntree);
   treeTraverse(syntree, symtabX);

   return syntree;
}

TreeNode *loadIOLib(TreeNode *syntree) 
{
   TreeNode *input, *output, *param_output;
   TreeNode *inputb, *outputb, *param_outputb;
   TreeNode *inputc, *outputc, *param_outputc;
   TreeNode *outnl;

   ///////// Stuff from next slides
   input = newDeclNode(FuncK, Integer);
   input->lineno = -1; // all are -1
   input->attr.name = strdup("input"); //We named the variables well
   input->type = Integer;
   inputb = newDeclNode(FuncK, Boolean);
   inputc = newDeclNode(FuncK, Boolean);
   param_output = newDeclNode(ParamK, Void);
   output = newDeclNode(FuncK, Void);
   param_outputb = newDeclNode(ParamK, Void);
   outputb = newDeclNode(FuncK, Void);
   param_outputc = newDeclNode(ParamK, Void);
   outputc = newDeclNode(FuncK, Void);
   outnl = newDeclNode(FuncK, Void);

   inputc = newDeclNode(FuncK, Boolean);
   inputc->type = Char;
   param_output = newDeclNode(ParamK, Void);
   param_output->attr.name = strdup("*dummy*");
   param_output->type = Integer;
   param_outputb = newDeclNode(ParamK, Void);
   param_outputb->attr.name = strdup("*dummy*");
   param_outputb->type = Boolean;
   param_outputc = newDeclNode(ParamK, Void);
   param_outputc->attr.name = strdup("*dummy*");
   param_outputc->type = Char;

   output = newDeclNode(FuncK, Void);
   output->child[0] = param_output;
   param_outputb = newDeclNode(ParamK, Void);
   outputb = newDeclNode(FuncK, Void);
   outputb->child[0] = param_outputb;
   param_outputc = newDeclNode(ParamK, Void);
   outputc = newDeclNode(FuncK, Void);
   outputc->child[0] = param_outputc;
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

void treeTraverse(TreeNode *syntree, SymbolTable *symtab)
{

}