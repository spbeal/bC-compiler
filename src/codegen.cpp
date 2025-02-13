#include <stdio.h>
#include "codegen.h"
#include "emitcode.h"
#include "semantics.h"
#include "scanType.h"
#include "parser.tab.h"

extern int numErrors;
extern int numWarnings;
// extern void yyparse();
extern int yydebug;
extern TreeNode *syntaxTree;
extern char **largerTokens;
extern void initTokenStrings();
// These offsets that never change
#define OFPOFF 0
#define RETURNOFFSET -1
int toffset; // next available temporary space
FILE *code; // shared global code – already included
static bool linenumFlag; // mark with line numbers
static int breakloc; // which while to break to
static SymbolTable *globals; // the global symbol tabl

void codegenExpression(TreeNode *currnode);
void codegenStatement(TreeNode *currnode);
void codegenDecl(TreeNode *currnode);
void codegenGeneral(TreeNode *currnode);


// Line Number
void commentLineNum(TreeNode *currnode)
{
   char buf[16];
   if (linenumFlag) {
   sprintf(buf, "%d", currnode->lineno);
   emitComment((char *)"Line: ", buf);
   }
}
// Assembly
void codegenLibraryFun(TreeNode *currnode)
{ 
   emitComment((char *)"");
   emitComment((char *)"** ** ** ** ** ** ** ** ** ** ** **");
   emitComment((char *)"FUNCTION", currnode->attr.name);
   // remember where this function is
   currnode->offset = emitSkip(0);
   // Store return address
   emitRM((char *)"ST", AC, RETURNOFFSET, FP, (char *)"Store return address");

   if (strcmp(currnode->attr.name, (char *)"input")==0) {
   emitRO((char *)"IN", RT, RT, RT, (char *)"Grab int input");
   }
   else if (strcmp(currnode->attr.name, (char *)"inputb")==0) {
   emitRO((char *)"INB", RT, RT, RT, (char *)"Grab bool input");
   }
   else if (strcmp(currnode->attr.name, (char *)"inputc")==0) {
   emitRO((char *)"INC", RT, RT, RT, (char *)"Grab char input");
   }
   else if (strcmp(currnode->attr.name, (char *)"output")==0) {
   emitRM((char *)"LD", AC, -2, FP, (char *)"Load parameter");
   emitRO((char *)"OUT", AC, AC, AC, (char *)"Output integer");
   }
   else if (strcmp(currnode->attr.name, (char *)"outputb")==0) {
   emitRM((char *)"LD", AC, -2, FP, (char *)"Load parameter");
   emitRO((char *)"OUTB", AC, AC, AC, (char *)"Output bool");
   }else if (strcmp(currnode->attr.name, (char *)"outputc")==0) {
   emitRM((char *)"LD", AC, -2, FP, (char *)"Load parameter");
   emitRO((char *)"OUTC", AC, AC, AC, (char *)"Output char");
   }
   else if (strcmp(currnode->attr.name, (char *)"outnl")==0) {
   emitRO((char *)"OUTNL", AC, AC, AC, (char *)"Output a newline");
   }
   else {
   emitComment((char *)"ERROR(LINKER): No support for special function");
   emitComment(currnode->attr.name);
   }

   emitRM((char *)"LD", AC, RETURNOFFSET, FP, (char *)"Load return address");
   emitRM((char *)"LD", FP, OFPOFF, FP, (char *)"Adjust fp");
   emitGoto(0, AC, (char *)"Return");
   emitComment((char *)"END FUNCTION", currnode->attr.name);
}

// process functions
// This generates a standard closing and returns.
void codegenFun(TreeNode *currnode)
{
   emitComment((char *)"");
   emitComment((char *)"** ** ** ** ** ** ** ** ** ** ** **");
   emitComment((char *)"FUNCTION", currnode->attr.name);
   toffset = currnode->size; // recover the end of activation record
   emitComment((char *)"TOFF set:", toffset);

   // IMPORTANT: For function nodes the offset is defined to be the position of the
   // function in the code space! This is accessible via the symbol table.
   // remember where this function is:
   currnode->offset = emitSkip(0); // offset holds the instruction address!!
   // Store return address
   emitRM((char *)"ST", AC, RETURNOFFSET, FP, (char *)"Store return address");

   // Generate code for the statements...
   codegenGeneral(currnode->child[1]);
   // In case there was no return statement
   // set return register to 0 and return
   emitComment((char *)"Add standard closing in case there is no return statement");
   emitRM((char *)"LDC", RT, 0, 6, (char *)"Set return value to 0");
   emitRM((char *)"LD", AC, RETURNOFFSET, FP, (char *)"Load return address");
   emitRM((char *)"LD", FP, OFPOFF, FP, (char *)"Adjust fp");
   emitGoto(0, AC, (char *)"Return");
   emitComment((char *)"END FUNCTION", currnode->attr.name);
}

// IdK AssignK VarK
int offsetRegister(VarKind v) {
   switch (v) {
      case Local: return FP;
      case Parameter: return FP;
      case Global: return GP;
      case LocalStatic: return GP;
      default:
         printf((char *)"ERROR(codegen): looking up offset register for a variable of type %d\n", v);
         return 666; // not the devil, intended to be getting a demon as a phone number thing
   }
}

void codegenExpression(TreeNode * currnode)
{
   // local state to remember stuff
   int skiploc=0, skiploc2=0, currloc=0; // some temporary instuction addresses
   TreeNode *loopindex=NULL; // a pointer to the index variable declaration node
   commentLineNum(currnode);
   
   // emitComment((char *)"EXPRESSION");
   switch (currnode->kind.exp) {
   /////////////////Other cases
      //emitComment((char *)"TOFF set:", toffset);
      case OpK: {
         if (currnode->child[0]) codegenExpression(currnode->child[0]);
         if (currnode->child[1]) {
            emitRM((char *)"ST", AC, toffset, FP, (char *)"Push left side");
            toffset--; emitComment((char *)"TOFF dec:", toffset);
            codegenExpression(currnode->child[1]);
            toffset++; emitComment((char *)"TOFF inc:", toffset);
            emitRM((char *)"LD", AC1, toffset, FP, (char *)"Pop left into ac1");
         }
            // More code here
         switch (currnode->attr.op)
         {
            case '+': emitRO((char *)"ADD", AC, AC1, AC, (char*)"Op +");break;
            case '-': emitRO((char *)"SUB", AC, AC1, AC, (char*)"Op -");break;
            case '/': emitRO((char *)"DIV", AC, AC1, AC, (char*)"Op /");break;
            case '*': emitRO((char *)"MUL", AC, AC1, AC, (char*)"Op *");break;
            case '<': emitRO((char *)"TLT", AC, AC1, AC, (char*)"Op <");break;
            case '>': emitRO((char *)"TGT", AC, AC1, AC, (char*)"Op >");break;
            case '?': emitRO((char *)"TLT", AC, AC1, AC, (char*)"Op ?");break;
            case MIN: emitRO((char *)"MIN", AC, AC1, AC, (char*)"Op :<:");break;
            case MAX: emitRO((char *)"MAX", AC, AC1, AC, (char*)"Op :>:");break;
            case SIZEOF: emitRO((char *)"SIZEOF", AC, AC1, AC, (char*)"Op SIZEOF");break;
            case CHSIGN: emitRO((char *)"CHSIGN", AC, AC1, AC, (char*)"Op CHSIGN");break;
            case AND: emitRO((char *)"AND", AC, AC1, AC, (char*)"Op AND");break;
            case NOT: emitRO((char *)"NOT", AC, AC1, AC, (char*)"Op NOT");break;
            case OR: emitRO((char *)"OR", AC, AC1, AC, (char*)"Op OR");break;
            case NEQ: emitRO((char *)"NEQ", AC, AC1, AC, (char*)"Op NEQ");break;
            case GEQ: emitRO((char *)"GEQ", AC, AC1, AC, (char*)"Op GEQ");break;
            case LEQ: emitRO((char *)"LEQ", AC, AC1, AC, (char*)"Op LEQ");break;
            case EQ: emitRO((char *)"EQ", AC, AC1, AC, (char*)"Op EQ");break;
            break;
         }
         break;
      }
      case AssignK: {
         TreeNode *lhs = currnode->child[0]; 
         int offReg;
         TreeNode *rhs = currnode->child[1];

         if (lhs->attr.op == '[') {
            // stuff
            lhs->isArray = true;
            TreeNode *var = lhs->child[0]; 
            offReg = offsetRegister(var->varKind);
            loopindex = lhs->child[1];

            // if var or loopIndex == NULL

            codegenExpression(loopindex);

            // Push index, Pop Index of array. codegen the value from index
            if (rhs != NULL) {
               emitRM((char *)"ST", AC, toffset, FP, (char *)"Push index");
               toffset--;
               emitComment((char *)"TOFF dec:", toffset);

               codegenExpression(rhs);
               // LDC then NEG for example. 

               toffset++;
               emitComment((char *)"TOFF inc:", toffset);
               emitRM((char *)"LD", AC1, toffset, FP, (char *)"Pop index");
            }

            if (var->varKind == Parameter)
            {

            }

            switch (currnode->attr.op)
            {
               case ADDASS:
               {
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"ADD", AC, AC1, AC, (char *)"op +=");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               }
               case MULASS:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"MUL", AC, AC1, AC, (char *)"op *=");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case DIVASS:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"DIV", AC, AC1, AC, (char *)"op /=");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case SUBASS:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"SUB", AC, AC1, AC, (char *)"op -=");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case DEC:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRM((char *)"LDA", AC, -1, AC, (char *)"decrement value of", lhs->attr.name);                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case INC:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRM((char *)"LDA", AC, 1, AC, (char *)"increment value of", lhs->attr.name);
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case '=':
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
            }
         }
         else
         {
            int offReg;
            offReg = offsetRegister(lhs->varKind);

            if (rhs) {
               codegenExpression(rhs);
            }

            switch (currnode->attr.op)
            {
               case ADDASS:
               {
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"ADD", AC, AC1, AC, (char *)"op +=");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               }
               case MULASS:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"MUL", AC, AC1, AC, (char *)"op *=");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case DIVASS:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"DIV", AC, AC1, AC, (char *)"op /=");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case SUBASS:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"SUB", AC, AC1, AC, (char *)"op -=");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case DEC:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"DEC", AC, AC1, AC, (char *)"decrement value of");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case INC:
                  emitRM((char *)"LD", AC1, lhs->offset, offReg,
                  (char *)"load lhs variable", lhs->attr.name);
                  emitRO((char *)"INC", AC, AC1, AC, (char *)"increment value of");
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
               case '=':
                  emitRM((char *)"ST", AC, lhs->offset, offReg,
                  (char *)"Store variable", lhs->attr.name);
                  break;
            }
            // Lots of cases that use it. Here is a sample:

               // emitRM((char *)"LD", AC1, lhs->offset, offReg,
               // (char *)"load lhs variable", lhs->attr.name);
               // emitRO((char *)"ADD", AC, AC1, AC, (char *)"op +=");
               // emitRM((char *)"ST", AC, lhs->offset, offReg,
               // (char *)"Store variable", lhs->attr.name);
               // break;
         }
         break;
      }
      // DOne
      case ConstantK: 
      {
         switch (currnode->type)
         {
            case Char:
               if (currnode->isArray) {
                  emitStrLit(currnode->offset, currnode->attr.string);
                  emitRM((char *)"LDA", AC, currnode->offset, 0, (char *)"Load address of char array");
               }
               else {
                  emitRM((char *)"LDC", AC, int(currnode->attr.cvalue), 6, (char *)"Load char constant");
               }
               break;
            case Integer:
            {
               emitRM((char *)"LDC", AC, int(currnode->attr.cvalue), 6, (char *)"Load integer constant");
               break;
            }
            case Boolean:
            {
               emitRM((char *)"LDC", AC, int(currnode->attr.cvalue), 6, (char *)"Load boolean constant");
               break;
            }
            break;
         }
         break;
      }
      case CallK: {
         emitComment((char*)"CALL", currnode->attr.name);
         if (currnode->child[1]) {
               // emitComment((char *)"END FUNCTION", currnode->attr.name);
            emitRM((char *)"ST", AC, toffset, FP, (char *)"Store fp in ghost frame for", currnode->attr.name);
            toffset--; emitComment((char *)"TOFF dec:", toffset);
            codegenExpression(currnode->child[1]);
            toffset++; emitComment((char *)"TOFF inc:", toffset);
            emitRM((char *)"LD", AC1, toffset, FP, (char *)"Pop left into ac1");
         }
         emitComment((char*)"Call end", currnode->attr.name);
         emitComment((char *)"TOFF set:", toffset);
         break;
      }
      case IdK: {
         if (currnode->isArray)
         {
            int offset = offsetRegister(currnode->varKind);
            if (currnode->varKind == Parameter)
               emitRM((char *)"LD", AC, currnode->offset, offset, (char *)"Load address of base of array", currnode->attr.name);
            else
               emitRM((char *)"LDA", AC, currnode->offset, offset, (char *)"Load address of base of array", currnode->attr.name);
         }
         else
         {
            int offset = offsetRegister(currnode->varKind);
            emitRM((char *)"LD", AC, currnode->offset, offset, (char *)"Load variable", currnode->attr.name);
         }
         break;
      }
      default: 
         break;
   }
}

void codegenStatement(TreeNode * currnode)
{
   commentLineNum(currnode);
   int savedToffset;
   int currloc = 0, skiploc = 0, skiploc2 = 0;

   switch (currnode->kind.stmt) {
      case CompoundK:
      { 
         int savedToffset;
         savedToffset = toffset;
         toffset = currnode->size; // recover the end of activation record
         emitComment((char *)"COMPOUND");
         emitComment((char *)"TOFF set:", toffset);
         codegenGeneral(currnode->child[0]); // process inits
         emitComment((char *)"Compound Body");
         codegenGeneral(currnode->child[1]); // process body
         toffset = savedToffset;
         emitComment((char *)"TOFF set:", toffset);
         emitComment((char *)"END COMPOUND");
         break;
      }
      case ForK:
      {
         emitComment((char *)"FOR");
         codegenGeneral(currnode->child[0]); 
         codegenGeneral(currnode->child[1]); 
         emitComment((char *)"END FOR");
         break;
      }
      case WhileK:
      {
         emitComment((char *)"WHILE");
         currloc = emitSkip(0); // keep top of loop
         codegenGeneral(currnode->child[0]); // process inits

         emitRM((char *)"JNZ", AC, 1, PC, (char *)"Jump to while part");
         emitComment((char *)"DO");

         /*
         Compound statement happens between these
         */

         skiploc = breakloc;
         breakloc = emitSkip(1); // keeps track of location of where the loop ends
         codegenGeneral(currnode->child[1]); // process body
         emitGotoAbs(currloc, (char*)"go to beginning of loop"); // JMP

         //backpatch
         backPatchAJumpToHere(breakloc, (char *)"Jump past loop [backpatch]"); // JMP

         // Keep track of for our break statement if it gets called later. 
         breakloc = skiploc;
         emitComment((char *)"END WHILE");
         break;
      }

      case IfK: {
         emitComment((char *)"IF");
         codegenGeneral(currnode->child[0]); 
         emitComment((char*) "THEN");
         codegenGeneral(currnode->child[1]); 
         emitComment((char *)"END IF");
         break;
      }
      //Done
      case ReturnK: {
         if (currnode->child[0] != NULL) {
            codegenExpression(currnode->child[0]);
            //emitRM((char *)"LDC", AC1, currnode->size-1, 6, (char *)"Load", currnode->type, "constant");
            emitRM((char *)"LDA", 2, 0, AC, (char *)"Copy result to return register");
         }
         emitRM((char *)"LD", AC, -1, FP, (char *)"Load return address");
         emitRM((char *)"LD", FP, GP, FP, (char *)"Adjust FP");
         //emitRO((char *)"JMP", AC1, GP, AC1, (char *)"Return");
         emitGoto(GP, AC, (char*)"Return");
         break;
      }
      // Done
      case BreakK: {
         emitComment((char *)"BREAK");
         // JMP SOMEWHERE ELSE
         emitGotoAbs(breakloc, (char*)"break");
         break;
      }
      case RangeK: {
         break;
      }
      default:
         break;
   }
   // end
}

// Complete
void codegenDecl(TreeNode *currnode)
{ 
   commentLineNum(currnode);
   switch(currnode->kind.decl) {
      // Done
      case VarK:
      {
         if (currnode->isArray) {
            switch (currnode->varKind) {
               case Local:
                  emitRM((char *)"LDC", AC, currnode->size-1, 6, (char *)"load size of array", currnode->attr.name);
                  emitRM((char *)"ST", AC, currnode->offset+1, offsetRegister(currnode->varKind),
                  (char *)"save size of array", currnode->attr.name);
                  break;
               case LocalStatic:
               case Parameter:
               case Global:
               // do nothing here
                  break;
               case None:
               // Error Condition
               break;
            }
               // ARRAY VALUE initialization
            }
            if (currnode->child[0]) {
               codegenExpression(currnode->child[0]);
               emitRM((char *)"LDA", AC1, currnode->offset, offsetRegister(currnode->varKind), (char *)"address of lhs");
               emitRM((char *)"LD", AC2, 1, AC, (char *)"size of rhs");
               emitRM((char *)"LD", AC3, 1, AC1, (char *)"size of lhs");
               emitRO((char *)"SWP", AC2, AC3, 6, (char *)"pick smallest size");
               emitRO((char *)"MOV", AC1, AC, AC2, (char *)"array op =");
            }
         else { // !currnode->isArray
            // SCALAR VALUE initialization
            if (currnode->child[0]) {
               switch (currnode->varKind) {
                  case Local:
                     // compute rhs -> AC;
                     codegenExpression(currnode->child[0]);
                     // save it
                     emitRM((char *)"ST", AC, currnode->offset, FP, (char *)"Store variable", currnode->attr.name);
                  case LocalStatic:
                  case Parameter:
                  case Global:
                  // do nothing here
                  break;
                  case None:
                  ///Error condition!!!
                  break;
               }
            }
         }
         break;
      }
      // DONE
      case FuncK:
         if (currnode->lineno == -1) { // These are the library functions we just added
            codegenLibraryFun(currnode);
         }
         else {
            codegenFun(currnode);
         }
         break;
      // Done
      case ParamK:
         // IMPORTANT: no instructions need to be allocated for parameters here
         break;
   }
}
void codegenGeneral(TreeNode *currnode)
{ 
   while (currnode) {
      switch (currnode->nodekind) {
         case StmtK:
            codegenStatement(currnode);
            break;
         case ExpK:
            emitComment((char *)"EXPRESSION");
            codegenExpression(currnode);
            break;
         case DeclK:
            codegenDecl(currnode);
            break;
      }
      currnode = currnode->sibling;
   }
}

void initAGlobalSymbol(std::string sym, void *ptr)
{
   TreeNode *currnode;
   // printf("Symbol: %s\n", sym.c_str()); // dump the symbol table
   currnode = (TreeNode *)ptr;
   // printf("lineno: %d\n", currnode->lineno); // dump the symbol table
   if (currnode->lineno != -1) {
      if (currnode->isArray) {
         emitRM((char *)"LDC", AC, currnode->size-1, 6, (char *)"load size of array", currnode->attr.name);
         emitRM((char *)"ST", AC, currnode->offset+1, GP, (char *)"save size of array", currnode->attr.name);
      }
      if (currnode->kind.decl==VarK &&
      (currnode->varKind == Global || currnode->varKind == LocalStatic)) 
      {
         if (currnode->child[0]) {
            // compute rhs -> AC;
            codegenExpression(currnode->child[0]);
            // save it
            emitRM((char *)"ST", AC, currnode->offset, GP,
            (char *)"Store variable", currnode->attr.name);
         }
      }
   }
}

void initGlobalArraySizes()
{
   emitComment((char *)"INIT GLOBALS AND STATICS");
   globals->applyToAllGlobal(initAGlobalSymbol);
   emitComment((char *)"END INIT GLOBALS AND STATICS");
}

// Generate init code ...
void codegenInit(int initJump, int globalOffset)
{ 
   backPatchAJumpToHere(initJump, (char *)"Jump to init [backpatch]");
   emitComment((char *)"INIT");
   //OLD pre 4.6 TM emitRM((char *)"LD", GP, 0, 0, (char *)"Set the global pointer");
   emitRM((char *)"LDA", FP, globalOffset, GP, (char *)"set first frame at end of globals");
   emitRM((char *)"ST", FP, 0, FP, (char *)"store old fp (point to self)");
   initGlobalArraySizes();
   emitRM((char *)"LDA", AC, 1, PC, (char *)"Return address in ac");

   { // jump to main
      TreeNode *funcNode;
      funcNode = (TreeNode *)(globals->lookup((char *)"main"));
      if (funcNode) {
         emitGotoAbs(funcNode->offset, (char *)"Jump to main");
      }
      else {
         printf((char *)"ERROR(LINKER): Procedure main is not defined.\n");
         numErrors++;
      }
   }
   emitRO((char *)"HALT", 0, 0, 0, (char *)"DONE!");
   emitComment((char *)"END INIT");
}

void codegenHeader(char *srcFile)
{
   emitComment((char *)"bC compiler version bC-Su23");
   emitComment((char *)"File compiled: ", srcFile);
}

void codegen(FILE * codeIn, char* srcFile, TreeNode *syntaxTree, 
            SymbolTable *globalsIn, int globalOffset, bool linenumFlagIn)
{
   int initJump;
   code = codeIn;
   globals = globalsIn;
   linenumFlag = linenumFlagIn;
   breakloc = 0;
   initJump = emitSkip(1); // save a place for the jump to init
   codegenHeader(srcFile); // nice comments describing what is compiled
   codegenGeneral(syntaxTree); // general code generation including I/O library
   codegenInit(initJump, globalOffset); // generation of initialization for run
}