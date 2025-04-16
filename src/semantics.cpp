#include "semantics.h"
#include "parser.tab.h"

extern int numErrors;
extern int numWarnings;
static int goffset; // top of global space
static int foffset; // top of local space

static int new_scope = 0; // mark new scope
static int unique_var = 0; // var counter
static bool found_return = false; // mark true in ReturnK
static TreeNode * current_function = NULL; // function inside

//static bool validReturn = 0;

extern char *largerTokens[LASTTERM+1];
void used_warnings(std::string str, void * current);

void find_parameters(TreeNode *current, SymbolTable *symtab) {
   TreeNode *params = current->child[0];
   TreeNode *temp;

   while (params) {
      temp = params->sibling;
      params->sibling = NULL;
      treeTraverse(params, symtab);
      params->sibling = temp;
      params = params->sibling;
   }
}

bool insertError(TreeNode *current, SymbolTable *symtab);
void semanticError(TreeNode * current, char * message);

TreeNode *semanticAnalysis(TreeNode *syntree, SymbolTable *symtabX, int &globalOffset)
{
   syntree = loadIOLib(syntree);
   treeTraverse(syntree, symtabX);
   symtabX->applyToAll(used_warnings);

   TreeNode * check_main = (TreeNode *)symtabX->lookup("main");
   if (check_main && check_main->kind.decl == FuncK && check_main->child[0] == NULL);
   else {
      printf("LINKER ERROR: A function named 'main' with no parameters must be defined.\n");
      numErrors++;
   }

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
• If they don't match, we have an error for that.
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
   inputc = newDeclNode(FuncK, Char);
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

void call_errors(TreeNode *current, SymbolTable *symtab)
{
   TreeNode * tmp = (TreeNode *) symtab->lookup(current->attr.name);
   TreeNode *parameters = current->child[0];
   TreeNode *lookups = tmp->child[0];
   TreeNode * save;
   int index = 1;

   while (parameters && lookups)
   {
      save = parameters->sibling;
      parameters->sibling = NULL;
      treeTraverse(parameters, symtab);
      parameters->sibling = save;

      /////////////////////////
      if (parameters->type != lookups->type) 
      {
         printf("SEMANTIC ERROR(%d): Expecting %s in parameter %d of call to '%s' declared on line %d but got %s.\n",current->lineno, type_str(lookups->type, false, false), index, tmp->attr.name, tmp->lineno,type_str(parameters->type, false, false));
         numErrors++;
      }
      // Arrays
      if (lookups->isArray && !parameters->isArray) 
      {
         printf("SEMANTIC ERROR(%d): Expecting array in parameter %d of call to '%s' declared on line %d.\n",current->lineno, index, tmp->attr.name, tmp->lineno);
         numErrors++;
      } 
      else if (!lookups->isArray && parameters->isArray && parameters->attr.op != '[') 
      {
         printf("SEMANTIC ERROR(%d): Not expecting array in parameter %d of call to '%s' declared on line %d.\n",current->lineno, index, tmp->attr.name, tmp->lineno);
         numErrors++;
      }
      /////////////////////////

      parameters = parameters->sibling;
      lookups = lookups->sibling;
      index++;
   }
   if (parameters && !lookups) {
      printf("SEMANTIC ERROR(%d): Too many parameters passed for function '%s' declared on line %d.\n",current->lineno, current->attr.name, tmp->lineno);
      numErrors++;
   } else if (!parameters && lookups) {
      printf("SEMANTIC ERROR(%d): Too few parameters passed for function '%s' declared on line %d.\n",current->lineno, current->attr.name, tmp->lineno);
      numErrors++;
   }
}

void operator_errors(TreeNode *current, SymbolTable *symtab)
{
   if (current->child[0] == NULL){ printf("SYNTAX ERROR(%d): child 0 cannot be NULL\n", current->lineno); numErrors++; return;}
   
   int op = current->attr.op;  
   TreeNode * tmp = (TreeNode *)symtab->lookup(current->attr.name);   
   TreeNode * left = NULL;
   TreeNode * right = NULL;

   if (current->child[0] != NULL) { 
      if (current->child[0]->attr.op == '[') 
      {
         left = current->child[0];
      }
      else 
      {
         left = (TreeNode *)symtab->lookup(current->child[0]->attr.name);
      }
      if (left == NULL){ left = current->child[0]; }
      if (left->type == UndefinedType && !left->isArray) return;
   }
   if (current->child[1] != NULL)
   {
      right = (TreeNode *)symtab->lookup(current->child[1]->attr.name);
      if (right == NULL)
      {
         right = current->child[1];
      }
   }

   // ------------------------------------------------
   // ------------------------------------------------
   if (op == ADDASS || op == SUBASS || op == MULASS || op == DIVASS || 
      op == DEC || op == INC || op == MIN || op == MAX || op == '%' ||
      op == '/' || op == '+' || op == '-' || op == '*')
      {
         if (left->type != Integer || right->type != Integer)
         {
            if (left->type != Integer)
            {
               printf("SEMANTIC ERROR(%d): '%s' requires operands of type int but lhs is of %s.\n", current->lineno, largerTokens[op], type_str(left->type, false, false));
               numErrors++;
            }
            if (right->type != Integer)
            {
               printf("SEMANTIC ERROR(%d): '%s' requires operands of type int but rhs is of %s.\n", current->lineno, largerTokens[op], type_str(right->type, false, false));
               numErrors++;
            }
         }
         if (((left->isArray && left->attr.op != '[') || (right->isArray && right->attr.op != '[')))
         {
            printf("SEMANTIC ERROR(%d): '%s' The operation '%s' does not work with arrays.\n", current->lineno, largerTokens[op]);
            numErrors++;
         }
      }
   // ------------------------------------------------
   else if ((op == AND || op == OR || op == NOT ))
   {
      if (left->type != Boolean || right->type != Boolean)
      {
         if (left->type != Boolean)
         {
            printf("SEMANTIC ERROR(%d): '%s' requires operands of type bool but lhs is of %s.\n", current->lineno, largerTokens[op], type_str(left->type, false, false));
            numErrors++;
         }
         if (right->type != Boolean)
         {
            printf("SEMANTIC ERROR(%d): '%s' requires operands of type bool but rhs is of %s.\n", current->lineno, largerTokens[op], type_str(right->type, false, false));
            numErrors++;
         }
      }
      if (((left->isArray && left->attr.op != '[') || (right->isArray && right->attr.op != '[')))
      {
         printf("SEMANTIC ERROR(%d): '%s' The operation '%s' does not work with arrays.\n", current->lineno, largerTokens[op]);
         numErrors++;
      }
   }
   // ------------------------------------------------
   else if (op == '[') 
   { 
      if (!left->isArray || left->type == UndefinedType) {
         printf("SEMANTIC ERROR(%d): Cannot index nonarray '%s'.\n", current->lineno, left->attr.name);
         numErrors++;
      }
      else if (right->type != Integer) {
         printf("SEMANTIC ERROR(%d): Array '%s' should be indexed by type int but got %s.\n", current->lineno, right->attr.name, type_str(right->type, false, false));
         numErrors++;
      }
      if (right->isArray) {
         printf("SEMANTIC ERROR(%d): Array index is the unindexed array '%s'.\n", current->lineno, right->attr.name);
         numErrors++;
      }
   }
   // ------------------------------------------------
   else if ( op == INC || op == DEC || op == '?' || op == CHSIGN) 
   {
      if (left->isArray && left->attr.op != '[') {
         printf("SEMANTIC ERROR(%d): The operation '%s' does not work with arrays.\n", current->lineno, largerTokens[op]);
        numErrors++;
      } 
      if (left->type != Integer) {
         printf("SEMANTIC ERROR(%d): Unary '%s' requires an operand of type int but was given %s.\n",  current->lineno, largerTokens[op], type_str(left->type, false, false));
         numErrors++;
      }
   }
   // ------------------------------------------------
   else if (op == '=' || op == EQ || op == NEQ || op == '>' || op == '<' || op == GEQ || op == LEQ) 
   {
      printf("%s", right->type);
      if (left->type != right->type) {
         printf("SEMANTIC ERROR(%d): '%s' requires operands of the same type but lhs is %s and rhs is %s.\n", current->lineno, largerTokens[op], type_str(left->type, false, false), type_str(right->type, false, false));
         numErrors++;
      }
      if (left->isArray && !right->isArray && left->attr.op != '[' ) {
         printf("SEMANTIC ERROR(%d): '%s' requires both operands be arrays or not but lhs is an array and rhs is not an array.\n", current->lineno, largerTokens[op]);
         numErrors++;
      } 
      else if (!left->isArray && right->isArray && right->attr.op != '[') 
      {
         printf("SEMANTIC ERROR(%d): '%s' requires both operands be arrays or not but lhs is not an array and rhs is an array.\n", current->lineno, largerTokens[op]);
         numErrors++;
      }      
   }
   // ------------------------------------------------
   else if (op == SIZEOF) if (!left->isArray) printf("SYNTAX ERROR(%d): The operation 'sizeof' only works with arrays.\n", current->lineno);
   // ------------------------------------------------
}

void decl_traverse(TreeNode * current, SymbolTable *symtab) {
   TreeNode * tmp;
   new_scope = 1;
   
   switch (current->kind.decl) {
      case VarK: {
         treeTraverse(current->child[0], symtab);
         if (current->child[0] != NULL) 
         {
            if (current->type != current->child[0]->type) {
               printf("SEMANTIC ERROR(%d): Initializer for variable '%s' of %s is of %s\n", current->lineno, current->attr.name, type_str(current->type, false, false), type_str(current->child[0]->type, false, false));
               numErrors++;
            }
            if (current->child[0]->kind.exp != ConstantK) 
            {
               printf("SEMANTIC ERROR(%d): Initializer for variable '%s' is not a constant expression.\n", current->lineno, current->attr.name);
               numErrors++;
            }
            if (current->isArray)
            {
               if (!current->child[0]->isArray)
               {
                  printf("SEMANTIC ERROR(%d): Initializer for variable '%s' requires both operands be arrays or not but variable is an array and rhs is not an array.\n",current->lineno, current->attr.name);
                  numErrors++;
               }
            }
            else
            {
               if (current->child[0]->isArray)
               {
                  printf("SEMANTIC ERROR(%d): Initializer for variable '%s' requires both operands be arrays or not but variable is not an array and rhs is an array.\n",current->lineno, current->attr.name);
                  numErrors++;                  
               }
            }
         }
         // Handle VarK in ParamK
      } // No break
      case ParamK: {
         if (current->child[0] != NULL) current->isAssigned = true; 
         if (insertError(current, symtab))
         {
            if (symtab->depth()==1) 
            {
               current->varKind = Global;
               current->offset = goffset;
               goffset -= current->size;
            }
            else if (current->isStatic)
            {
               current->varKind = LocalStatic;
               current->offset = goffset;
               goffset -= current->size;

               // 1 billion variable max;
               char * name;
               char * num_str = new char[12];
               name = new char[strlen(current->attr.name) + 12];
               name[0] = '\0';
               strcat(name, current->attr.name);
               strcat(name, "_");
               sprintf(num_str, "%d", unique_var++);
               strcat(name, num_str);
               symtab->insertGlobal(name, current);
               delete [] name;
               delete [] num_str;
            }
            else 
            {
               current->varKind = Local;
               current->offset = foffset;
               foffset -= current->size; 
            }
         }
         else
         {
            tmp = (TreeNode *)symtab->lookup(current->attr.name);
            printf("SEMANTIC ERROR(%d): Symbol '%s' is already declared at line %d.\n", current->lineno, current->attr.name, tmp->lineno);
            numErrors++;
         }
         // Check at end for parameter.
         if (current->kind.decl == ParamK) current->varKind = Parameter;
         else if (current->isArray) current->offset--;
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);

         break;
      }
      case FuncK: {
         current->varKind = Global;
         foffset = -2;
         new_scope = 0; // reset scope
         
         // Insert into the symbol table
         bool insert = insertError(current, symtab);
         if (insert == false)
         {
            tmp = (TreeNode *)symtab->lookup(current->attr.name);
            printf("SEMANTIC ERROR(%d): Symbol '%s' is already declared at line %d.\n", current->lineno, current->attr.name, tmp->lineno);
            numErrors++;
         }

         // Check for main, check for return.
         symtab->enter(current->attr.name);
         current_function = current;
         //-------------------------------------
         treeTraverse(current->child[0], symtab);
            current->size = foffset;
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);

         symtab->applyToAll(used_warnings);
         if (found_return == false && current->lineno != -1 && current->type != Void) 
         {
            printf("SEMANTIC WARNING(%d): Expecting to return %s but function '%s' has no return statement.\n",current->lineno, type_str(current->type, false, false), current->attr.name);
            numWarnings++;
         }
         //-------------------------------------
         symtab->leave();

         found_return = false;
         break;
      }
      default: break;
   }
}

void stmt_traverse(TreeNode * current, SymbolTable *symtab) {
   if (current->kind.stmt != CompoundK) new_scope = 1;
   TreeNode * tmp = NULL;
   int rememberFoffset;

   switch (current->kind.stmt) {
      case IfK: {
         if (new_scope)
         {
            symtab->enter((char *)"IfStmt");
            rememberFoffset = foffset;
            treeTraverse(current->child[0], symtab);

            if (current->child[0]) tmp = (TreeNode *)symtab->lookup(current->child[0]->attr.name);
            if (tmp == NULL) tmp = current->child[0];
            else
            {
               if (tmp->type != Boolean && tmp->type != UndefinedType) 
               {
                  printf("SEMANTIC ERROR(%d): Expecting Boolean test condition in if statement but got %s.\n", current->lineno, type_str(tmp->type, false, false));
                  numErrors++;
               }
               if (tmp->isArray)
               {
                  printf("SEMANTIC ERROR(%d): Cannot use array as test condition in if statement.\n", current->lineno);
                  numErrors++;
               }
            }

            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
            foffset = rememberFoffset;
            symtab->leave();
         }
         else
         {
            new_scope = 1;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
         }
         break;
      }
      case WhileK: {
         if (new_scope)
         {
            symtab->enter((char *)"WhileStmt");
            rememberFoffset = foffset;
            treeTraverse(current->child[0], symtab);

            // Errors
            if (current->child[0])
            {
               if (current->child[0]->type != Boolean && current->child[0]->type != UndefinedType) 
               {
                  printf("SEMANTIC ERROR(%d): Expecting Boolean test condition in while statement but got %s.\n", current->lineno, type_str(current->child[0]->type, false, false));
                  numErrors++;
               }

               if (current->child[0]->isArray) 
               {
                  printf("SEMANTIC ERROR(%d): Cannot use array as test condition in while statement.\n", current->lineno);
                  numErrors++;
               }
            }

            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
            foffset = rememberFoffset;
            symtab->leave();
         }
         else
         {
            new_scope = 1;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
         }
         break;
      }
      case ForK: {
         if (new_scope)
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
            new_scope = 1;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
         }
         break;
      }
      case CompoundK: {
         if(new_scope) 
         {
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
            new_scope = 1;
            treeTraverse(current->child[0], symtab);
            current->size = foffset;
            treeTraverse(current->child[1], symtab);
            treeTraverse(current->child[2], symtab);
         }

         // IF AFTER FUNCTION TREAT DIFFERENTLY
         break;
      }
      case ReturnK: {
         found_return = true;
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);

         if (current->child[0] != NULL)
         {
            tmp = (TreeNode *) symtab->lookup(current->child[0]->attr.name);
            if (current->child[0]->type == UndefinedType);
            else if (tmp != NULL && tmp->isArray)
            {
               printf("SEMANTIC ERROR(%d): Cannot return an array.\n", current->lineno);
               numErrors++;               
            }
            else if (current_function != NULL && current_function->type != current->child[0]->type)
            {
               if (current_function->type != Void) printf("SEMANTIC ERROR(%d): Function '%s' at line %d is expecting to return %s but returns %s.\n",current->lineno, current_function->attr.name, current_function->lineno, type_str(current_function->type, false, false), type_str(current->child[0]->type, false, false));
               else printf("SEMANTIC ERROR(%d): Function '%s' at line %d is expecting no return value, but return has a value.\n", current->lineno, current_function->attr.name, current_function->lineno);
               numErrors++;
            }
         }
         else if (current_function != NULL) // child is null
         {
            printf("SEMANTIC ERROR(%d): Function '%s' at line %d is expecting to return %s but return has no value.\n",current->lineno, current_function->attr.name, current_function->lineno, type_str(current_function->type, false, false));
            numErrors++;
         }

         break;
      }
      case BreakK: {
         // This means within the file and main.
         if (symtab->depth() <= 2) 
         {
            printf("SEMANTIC ERROR(%d): Cannot have a break statement outside of loop.\n", current->lineno);
            numErrors++;
         }
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);

         break;
      }
      case RangeK: {
         for (int i = 0; i < 3; i++) // MAXCHILDREN
         {
            if (current->child[i] != NULL) 
            {
               if (current->child[i]->type != Integer) 
               {
                  printf("SEMANTIC ERROR(%d): Expecting type int in position %d in range of for statement but got %s.\n",current->lineno, i+1, type_str(current->child[i]->type, false, false));
                  numErrors++;
               }
               if (current->child[i]->isArray) 
               {
                  printf("SEMANTIC ERROR(%d): Cannot use array in position %d in range of for statement.\n",current->lineno, i+1);
                  numErrors++;
               }
            }
         }
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);

         break;
      }
      default: break;
   }
}

void exp_traverse(TreeNode * current, SymbolTable *symtab) {
   TreeNode * tmp;
   new_scope = 1;

   switch (current->kind.exp) {
      case AssignK: {
         // Just like op
         // current->type = Integer;
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);
         operator_errors(current, symtab);

         int op = current->attr.op;  
         switch (op)
         {
            case AND: case NOT: case OR: current->type = Boolean;
            case '=': case '[': 
               current->type = current->child[0]->type;
               if (current->child[0]->type != UndefinedType) 
                  current->type = current->child[0]->type;
               break;
            case EQ: case NEQ: case LEQ: case GEQ: case '<': case '>':
               current->type = Boolean;
               break;
            default:
               break;
         }
         // if (current->child[0] == NULL) {} else {
         //    tmp = (TreeNode *) symtab->lookup(current->child[0]->attr.name);

         //    if (tmp == NULL) {
         //       current->type = current->child[0]->type;
         //       current->child[0]->isAssigned = true;
         //    } else {
         //       tmp->isAssigned = true;
         //       current->type = tmp->type;
         //    }

         // }

         break;
      }
      case OpK: {
         //current->varKind = Local;
         // Check the children because Ops require children
         // current->type = Integer;
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);
         int op = current->attr.op;  

         // Check all operators
         if (op == GEQ || op == LEQ || op == NEQ || op == '<' || op == '>'
             || op == AND || op == NOT || op == OR  || op == EQ)
            current->type = Boolean;
         else if (op == '=' || op == '[')
            current->type = current->child[0]->type;
            if (op == '[') current->isArray = true;
         else if (op == ADDASS || op == SUBASS || op == MULASS || op == DIVASS || 
             op == DEC || op == INC || op == MIN || op == MAX || op == '%' ||
             op == '/' || op == '?' || op == '+' || op == '-' || op == '*')
            current->type = Integer;
         else if (op == SIZEOF) current->type = Integer;
         else 
         {
            if (current->child[0] != NULL)
            {
               // tmp = (TreeNode *)symtab->lookup(current->child[0]->attr.name);
               // if (tmp == NULL) {
               //    current->type = current->child[0]->type;
               // } else {
               //    current->type = tmp->type;
               // }
            }
            else
            {
               printf("ERROR: Op child can not be NULL - semantics.cpp::treeExpTraverse\n");
               numErrors++;
            }
         }
         operator_errors(current, symtab);
         break;
      }
      case ConstantK: {
         if (current->type == Char && current->isArray) { // Deal with strings
            current->varKind = Global;
            current->offset = goffset - 1;
            goffset -= current->size;
         }
         break;
      }
      case CallK: {
         // only ever need a single child for the ID
         treeTraverse(current->child[0], symtab);
         tmp = (TreeNode *)(symtab->lookup(current->attr.name));
         if (tmp) 
         {
            if (tmp->kind.decl != FuncK)
            {
               printf("SEMANTIC ERROR(%d): '%s' is a simple variable and cannot be called.\n",current->lineno, current->attr.name);
               numErrors++;
            }
            current->type = tmp->type;
            current->offset = tmp->offset; 
            current->isUsed = true;
            tmp->isUsed = true;
            call_errors(current, symtab);
         }
         else
         {
            printf("SEMANTIC ERROR(%d): Symbol '%s' is not declared.\n", current->lineno, current->attr.name);
            current->type = UndefinedType;
            numErrors++;
         }

         break;
      }
      case IdK: {
         tmp = (TreeNode *)(symtab->lookup(current->attr.name)); // Look up in the symbol table
         if (tmp == NULL) {
            printf("SEMANTIC ERROR(%d): Symbol \'%s\' is not declared.\n", current->lineno, current->attr.name);
            current->type = UndefinedType;
            numErrors++;
         }

         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);

         if (tmp != NULL)
         {
            if (tmp->kind.decl == FuncK) {
               printf("SEMANTIC ERROR(%d): Cannot use function '%s' as a variable.\n", current->lineno, tmp->attr.name);
               numErrors++;
            }
            if (!tmp->isAssigned && !tmp->isArray && tmp->kind.decl == VarK) {
               printf("SEMANTIC WARNING(%d): Variable '%s' may be uninitialized when used here.\n", current->lineno, tmp->attr.name);
               tmp->isAssigned = true;
               numWarnings++;
            }

            current->isUsed = true;
            tmp->isUsed = true;
            current->type = tmp->type;
            current->isStatic = tmp->isStatic;
            current->isArray = tmp->isArray;
            current->size = tmp->size;
            current->varKind = tmp->varKind;
            current->offset = tmp->offset;
         }
         break;
      }
      default: 
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);
         break;
   }
}

void used_warnings(std::string str, void * curr) {
   TreeNode* current = (TreeNode *) curr;

   // isUsed must be false for duplicates
   if (current && current->isUsed == false && current->lineno != -1) {
      switch (current->kind.decl) {
         case VarK: {
            char *dash = strchr(current->attr.name, '-');
            if (dash != NULL) *dash = '\0'; // truncate for expressions
            printf("SEMANTIC WARNING(%d): The variable '%s' seems not to be used.\n",current->lineno, current->attr.name);
            break;           
         }
         case ParamK:
            printf("SEMANTIC WARNING(%d): The parameter '%s' seems not to be used.\n",current->lineno, current->attr.name);
            break;
         case FuncK:
            // If main we ignore
            if (strcmp(current->attr.name, "main") == 0) {
               return;
            }
            printf("SEMANTIC WARNING(%d): The function '%s' seems not to be used.\n",current->lineno, current->attr.name);
            break;

      }
      current->isUsed = true;
      numWarnings++;
   }
}

ExpType get_return_type(char * name, SymbolTable * symtab)
{
   TreeNode *funcNode = (TreeNode *)(symtab->lookup(name));

   if (funcNode != NULL) {
      return funcNode->type;
   }
   return Void;
}

bool insertError(TreeNode *current, SymbolTable *symtab) {
   if (symtab->insert(current->attr.name, current)) {
      return true;
   }
   return false;
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