#include "semantics.h"
#include "parser.tab.h"

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

void decl_traverse(TreeNode * current, SymbolTable *symtab) {
   TreeNode * tmp;
   

   switch (current->kind.decl) {
      case VarK: {
         // current->varKind = Local;
         // current->varKind = LocalStatic;
         // current->varKind = Global;
         if (current->isArray) current->offset--; // pt to array after size
         // pt to array after size.

         if (symtab->depth()==1) // This is a global variable since it is not in a function
         // Set the varKind to Global for VarK (Parameter for ParamK)
         current->offset = goffset;
         goffset -= current->size;
         
         // Otherwise, if current->isStatic // This is a static variable
         // Set varKind to LocalStatic for VarK (Parameter for ParamK)
         current->offset = goffset;
         goffset -= current->size;
         //symtab->insertGlobal with a unique name. (Keep a static int variable and append it to the end of the name)
         
         // Otherwise, treat it as normal
         // Set varKind to Local for VarK (Parameter for ParamK)
         current->offset = foffset;
         foffset -= current->size; 

         break;
      }
      case ParamK: {
         current->varKind = Parameter;

         if (symtab->depth()==1) // This is a global variable since it is not in a function
         // Set the varKind to Global for VarK (Parameter for ParamK)
         current->offset = goffset;
         goffset -= current->size;
         
         // Otherwise, if current->isStatic // This is a static variable
         // Set varKind to LocalStatic for VarK (Parameter for ParamK)
         current->offset = goffset;
         goffset -= current->size;
         //symtab->insertGlobal with a unique name. (Keep a static int variable and append it to the end of the name)
         
         // Otherwise, treat it as normal
         // Set varKind to Local for VarK (Parameter for ParamK)
         current->offset = foffset;
         foffset -= current->size; 
         break;
      }
      case FuncK: {
         current->varKind = Global;
         foffset = -2;

         //Traverse child[0]’s tree (where the parameters are)
         current->size = foffset;
         //insert
         //lookup
         break;
      }
      default: break;
   }
}

void stmt_traverse(TreeNode * current, SymbolTable *symtab) {
   TreeNode * tmp;
   switch (current->kind.stmt) {
      case IfK: {
         current->varKind = Local;
         break;
      }
      case WhileK: {
         current->varKind = Local;
         break;
      }
      case ForK: {
         /*
         • symtab->enter((char *)"ForStmt");
         • Remember the current Offset
         • Traverse child[0]’s tree
         • foffset-=2; // Make space for the for loop var
         • current->size = foffset;
         • Traverse child[1]’s tree
         • Traverse child[2]’s tree
         • Restore the current Offset
         • symtab->leave();
         */
         current->varKind = Local;
         break;
      }
      case CompoundK: {
         /*
         CompoundK
         • If newScope
         • Remember the current Offset
         • Traverse child[0]’s tree
         • current->size = foffset;
         • Traverse child[1]’s tree
         • Restore the current Offset
         • Otherwise
         • Traverse child[0]’s tree
         • current->size = foffset;
         • Traverse child[1]’s tree
         */
         current->varKind = Local;
         symtab->enter((char *)"compoundStmt");
         treeTraverse(current->child[0], symtab); // process declarations
         // More stuff
         current->size = foffset;
         treeTraverse(current->child[1], symtab);
         // More stuff
         symtab->leave(); //end of processing of the compound statement

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

         /*
         int rememberFoffset;
         symtab->enter((char *)"compoundStmt");
         rememberFoffset = foffset;
         treeTraverse(current->child[0], symtab);
         current->size = foffset;
         treeTraverse(current->child[1], symtab);
         foffset = rememberFoffset;
         symtab->leave();
         */

         // IF AFTER FUNCTION TREAT DIFFERENTLY
         break;
      }
      case ReturnK: {
         current->varKind = Local;
         break;
      }
      case BreakK: {
         current->varKind = Local;
         break;
      }
      case RangeK: {
         current->varKind = Local;
         break;
      }
      default: break;
   }
}

void exp_traverse(TreeNode * current, SymbolTable *symtab) {
   TreeNode * tmp;

   switch (current->kind.exp) {
      case AssignK: {
         current->varKind = LocalStatic;
         current->varKind = Local;
         break;
      }
      case OpK: {
         current->varKind = Local;
         // Check the children because Ops require children
         //treeTraverse(tree->child[0], symtab);
         //treeTraverse(tree->child[1], symtab);
         //treeTraverse(tree->child[2], symtab);
         
         int op = current->attr.op;  
         if (op == '[') current->isArray = true;

         // Check all operators
         if (op == GEQ || op == EQ || op == LEQ || op == NEQ || op == '<' || op == '>')
            current->type = Boolean;
         else if (op == SIZEOF || op == CHSIGN)
            current->type = Integer;
         else 
         {
            if (current->child[0] != NULL)
            {
               // Returns void * default.
               tmp = (TreeNode*) symtab->lookup(current->child[0]->attr.name);
               if (tmp == NULL)
                  current->type = current->child[0]->type;
               else 
                  current->type = tmp->type;
            }
            else
            {
               printf("Error");
            }
         }

         break;
      }
      case CallK: {
         current->varKind = Local;
         // Similar to IdK, set type and size too
         if ((tmp = (TreeNode *)(symtab->lookup(current->attr.name)))) {
            current->type = tmp->type;
            current->isStatic = tmp->isStatic;
            current->isArray = tmp->isArray;
            current->size = tmp->size;
            current->varKind = tmp->varKind;
            current->offset = tmp->offset;
         }
         break;
      }
      case ConstantK: {
         if (current->type == Char && current->isArray) { // Deal with strings
            current->varKind = Global;
            current->offset = goffset - 1;
            goffset -= current->size;
         }
         //current->varKind = Local;
         break;
      }
      case IdK: {
         //tmp = (TreeNode *)(symtab->lookup(current->attr.name) // Look up in the symbol table
         // current->offset = tmp->offset;
         //current->varKind = Local;

         if ((tmp = (TreeNode *)(symtab->lookup(current->attr.name)))) {
            current->type = tmp->type;
            current->isStatic = tmp->isStatic;
            current->isArray = tmp->isArray;
            current->size = tmp->size;
            current->varKind = tmp->varKind;
            current->offset = tmp->offset;
         }

         break;
      }
      default: break;
   }
}

void treeTraverse(TreeNode * tree, SymbolTable *symtab) {
   if (tree == NULL) {
      return;
   }

   switch(tree->nodekind) {
      case DeclK: {
         decl_traverse(tree, symtab);
         break;
      }
      case StmtK: {
         stmt_traverse(tree, symtab);
         break;
      }
      case ExpK: {
         exp_traverse(tree, symtab);
         break;
      }
      default: break;
   }

   if (tree->sibling) {
      treeTraverse(tree->sibling, symtab);
   }
}