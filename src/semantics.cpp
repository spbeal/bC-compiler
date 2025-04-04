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

void operator_errors(TreeNode *current, SymbolTable *symtab)
{
   TreeNode * left = (TreeNode *)symtab->lookup(current->child[0]->attr.name);
   TreeNode * right = (TreeNode *)symtab->lookup(current->child[1]->attr.name);
   TreeNode * tmp = (TreeNode *)symtab->lookup(current->attr.name);
   if (current->child[0] == NULL){ printf("SYNTAX ERROR(%d): child 0 cannot be NULL\n", current->lineno); numErrors++; return;}
   else { 
      if (current->child[0]->attr.op == '[') 
      {
         left = current->child[0];
      }
      else 
      {
         if (left == NULL){ left = current->child[0]; }
      }
      if (left->type == UndefinedType && !left->isArray) return;
   }
   if (current->child[1] != NULL && right == NULL) { 
      right = current->child[1];
   }

   // ------------------------------------------------
   // ------------------------------------------------
   if (op == ADDASS || op == SUBASS || op == MULASS || op == DIVASS || 
      op == DEC || op == INC || op == MIN || op == MAX || op == '%' ||
      op == '/' || op == '+' || op == '-' || op == '*')
      {
         if (left->type || right->type != Integer)
         {
            if (left->type != Integer)
            {
               printf("SEMANTIC ERROR(%d): '%s' requires operands of type int but lhs is of %s.\n",current->lineno, largerTokens[op], type_str(left->type, false, false));
               numErrors++;
            }
            if (right->type != Integer)
            {
               printf("SEMANTIC ERROR(%d): '%s' requires operands of type int but rhs is of %s.\n",current->lineno, largerTokens[op], type_str(right->type, false, false));
               numErrors++;
            }
         }
         if (((left->isArray && left->attr.op != '[') || (right->isArray && right->attr.op != '[')))
         {
            printf("SEMANTIC ERROR(%d): '%s' The operation '%s' does not work with arrays.\n",current->lineno, largerTokens[op]);
            numErrors++;
         }
      }
   // ------------------------------------------------
   else if ((op == AND || op == OR || op == NOT ))
   {
      if (left->type || right->type != Boolean)
      {
         if (left->type != Boolean)
         {
            printf("SEMANTIC ERROR(%d): '%s' requires operands of type bool but lhs is of %s.\n",current->lineno, largerTokens[op], type_str(left->type, false, false));
            numErrors++;
         }
         if (right->type != Boolean)
         {
            printf("SEMANTIC ERROR(%d): '%s' requires operands of type bool but lhs is of %s.\n",current->lineno, largerTokens[op], type_str(right->type, false, false));
            numErrors++;
         }
      }
      if (((left->isArray && left->attr.op != '[') || (right->isArray && right->attr.op != '[')))
      {
         printf("SEMANTIC ERROR(%d): '%s' The operation '%s' does not work with arrays.\n",current->lineno, largerTokens[op]);
         numErrors++;
      }
   }
   // ------------------------------------------------
   else if (op == '[') { }
   // ------------------------------------------------
   else if (op == '?' || op == CHSIGN || op == INC || op == DEC) {}
   // ------------------------------------------------
   else if (op == '=' || op == EQ || op == NEQ || op == '>' || op == GEQ || op == '<' || op == LEQ) {}
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
         
         // Handle VarK in ParamK
      }
      case ParamK: {
         //treeTraverse(current->child[0], symtab);
         // This is a global variable since it is not in a function
         if (insertError(current, symtab))
         {
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
               sprintf(num_str, "%d", unique_var++);
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
         if (found_return == false && current->lineno != -1 && current->type != Void) {
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

   TreeNode * tmp;
   int rememberFoffset;
   switch (current->kind.stmt) {
      case IfK: {
         if (new_scope)
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
         }
         else if (current_function != NULL) // child is null
         {
            printf("SEMANTIC ERROR(%d): Function '%s' at line %d is expecting to return %s but return has no value.\n",current->lineno, current_function->attr.name, current_function->lineno, type_str(current_function->type, false, false));
            numErrors++;
         }
          

         break;
      }
      case BreakK: {
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);

         break;
      }
      case RangeK: {
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
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);
         int op = current->attr.op;  

         current->type = Integer;

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
         operator_errors(current, symtab);

         break;
      }
      case OpK: {
         //current->varKind = Local;
         // Check the children because Ops require children
         treeTraverse(current->child[0], symtab);
         treeTraverse(current->child[1], symtab);
         treeTraverse(current->child[2], symtab);
         current->type = Integer;
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
            if (current->child[0] != NULL);
            // {
            //    // Returns void * default.
            //    tmp = (TreeNode*) symtab->lookup(current->child[0]->attr.name);
            //    if (tmp == NULL)
            //       current->type = current->child[0]->type;
            //    else 
            //       current->type = tmp->type;
            // }
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
         else // not array, not char
         {
            //current->varKind = Global;
         }
         break;
      }
      case CallK: {
         // only ever need a single child for the ID
         treeTraverse(current->child[0], symtab);
         if (tmp = (TreeNode *)(symtab->lookup(current->attr.name))) {
            current->type = tmp->type;
            current->offset = tmp->offset; 
            //current->size = tmp->size;
            //find_parameters(current, symtab);
         }
         else
         {
            printf("Error 2");
            numErrors++;
         }

         //current->varKind = Local;

         // if (current->type == Void)
         // {
         //    TreeNode *funcNode = (TreeNode *)(symtab->lookup(current->attr.name));
         //    if (funcNode != NULL) current->type = funcNode->type;
         // }

         // treeTraverse(current->child[0], symtab);
         
         // tmp = (TreeNode *)(symtab->lookup(current->attr.name));
         // if (tmp != NULL) {
         //    // Check if its a function 
         //    // if not numErrors++;

         //    current->isUsed = true;
         //    tmp->isUsed = true;

         //    current->type = tmp->type;
         //    //current->isStatic = tmp->isStatic;
         //    //current->isArray = tmp->isArray;
         //    //current->size = tmp->size;
         //    //current->varKind = tmp->varKind;
         //    current->offset = tmp->offset;

         //    // Find all parameters
         //    // TreeNode * params = current->child[0];
         //    // TreeNode * temporary;
         //    // while (params) {
         //    //    temporary = params->sibling;
         //    //    params->sibling = NULL;
         //    //    treeTraverse(params, symtab);
         //    //    params->sibling = temporary;
         //    //    params = params->sibling;
         //    // }
         // }
         // else
         // {
         //    //current->type = Boolean;
         //    //printf("Error");
         //    numErrors++;
         // }
         
         //TreeNode *funcNode = (TreeNode *)(symtab->lookup(current->attr.name));

         // if (funcNode != NULL) {
         //    if (funcNode->kind.decl == FuncK) { // Ensure it's a function
         //          current->isUsed = true;
         //          funcNode->isUsed = true;

         //          // Set type to function return type
         //          current->type = funcNode->type;
         //    } else {
         //          printf("Error: '%s' is not a function at line %d\n", current->attr.name, current->lineno);
         //          numErrors++;
         //          current->type = Void; // Prevent further errors
         //    }
         // } else {
         //    printf("Error: Function '%s' not declared at line %d\n", current->attr.name, current->lineno);
         //    numErrors++;
         //    current->type = Void;
         // }

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
   // ERROR
   printf("Error 4");
   numErrors++;
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