#include "semantics.h"
#include "parser.tab.h"

// memory offsets are GLOBAL
static int goffset; // top of global space
static int foffset; // top of local space
static int newScope = 0; // mark new scope
static int uniqueVar = 0;
static int validReturn = 0;

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
         // treeTraverse(current->child[0], symtab);
         
         // Handle VarK in ParamK
      }
      case ParamK: {
         // This is a global variable since it is not in a function
         if (symtab->depth()==1) 
         {
            // Set the varKind to Global for VarK (Parameter for ParamK)
            //if (current->kind.decl == VarK) 
            current->varKind = Global;
            //else if (current->kind.decl == ParamK) current->type = Parameter;
            current->offset = goffset;
            goffset -= current->size;
         }
         // Otherwise, if current->isStatic // This is a static variable
         else if (current->isStatic)
         {
            // Set varKind to LocalStatic for VarK (Parameter for ParamK)
            //if (current->kind.decl == VarK) 
            current->varKind = LocalStatic;
            //else if (current->kind.decl == ParamK) current->type = Parameter;
            current->offset = goffset;
            goffset -= current->size;

            // 1 billion variable max;
            char * name;
            char * num_str = new char[12];
            name = new char[strlen(current->attr.name) + 12];
            name[0] = '\0';
            strcat(name, current->attr.name);
            strcat(name, "_");
            sprintf(num_str, "%d", uniqueVar++);
            strcat(name, num_str);
            symtab->insertGlobal(name, current);
            delete [] name;
            delete [] num_str;
         }
         else 
         {
            // Set varKind to Local for VarK (Parameter for ParamK)
            //if (current->kind.decl == VarK) 
            current->varKind = Local;
            //else if (current->kind.decl == ParamK) current->type = Parameter;
            current->offset = foffset;
            foffset -= current->size; 
         }
         // Check at end for parameter.
         if (current->kind.decl == ParamK) current->varKind = Parameter;
         else if (current->isArray) current->offset--;

         // treeTraverse(current->child[1], symtab);
         // treeTraverse(current->child[2], symtab);

         break;
      }
      case FuncK: {
         current->varKind = Global;
         foffset = -2;
         newScope = 0; // reset scope

         symtab->enter(current->attr.name);
         //rememberFoffset = foffset;

         treeTraverse(current->child[0], symtab);
         current->size = foffset;
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);

         //foffset = rememberFoffset;
         symtab->leave();

         //Traverse child[0]’s tree (where the parameters are)

         //insert
         //lookup
         break;
      }
      default: break;
   }
}

void stmt_traverse(TreeNode * current, SymbolTable *symtab) {
   if (current->kind.stmt != CompoundK) newScope = 1;

   TreeNode * tmp;
   int rememberFoffset;
   switch (current->kind.stmt) {
      case IfK: {
         if (newScope)
         {
            symtab->enter((char *)"IfStmt");
            rememberFoffset = foffset;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
            foffset = rememberFoffset;
            symtab->leave();
         }
         else
         {
            newScope = 1;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
         }
         break;
      }
      case WhileK: {
         if (newScope)
         {
            symtab->enter((char *)"WhileStmt");
            rememberFoffset = foffset;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
            foffset = rememberFoffset;
            symtab->leave();
         }
         else
         {
            newScope = 1;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
         }
         break;
      }
      case ForK: {
         if (newScope)
         {
            symtab->enter((char *)"ForStmt");
            rememberFoffset = foffset;
            treeTraverse(current->child[0], symtab);
            foffset-=2;
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
            foffset = rememberFoffset;
            symtab->leave();
         }
         else
         {
            newScope = 1;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
         }
         //current->varKind = Local;
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
         if(newScope) 
         {
            //char *id = strdup("{");
            //symtab->enter("NewScope from " + (std::string)id);
            symtab->enter((char *)"compoundStmt");
            rememberFoffset = foffset;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
            foffset = rememberFoffset;
            symtab->leave();
         }
         else
         {
            newScope = 1;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
         }

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
         if (current->isAssigned)
         {

         }
         else 
         {

         }
         //current->varKind = LocalStatic;
         //current->varKind = Local;
         break;
      }
      case OpK: {
         //current->varKind = Local;
         // Check the children because Ops require children
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);
         
         int op = current->attr.op;  
         //if (op == '[') current->isArray = true;

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
         else // not array, not char
         {
            current->varKind = Global;
         }
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