#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treeNodes.h"
#include "treeUtils.h"
#include "symbolTable.h"
#include "emitcode.h"

void codegen(FILE *codeIn,          // where the code should be written
             char *srcFile,         // name of file compiled
             TreeNode *syntaxTree,  // tree to process
	     SymbolTable *globalsIn,     // globals so function info can be found
	     int globalOffset,      // size of the global frame
             bool linenumFlagIn);   // comment with line numbers

#endif
