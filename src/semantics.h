#ifndef _SEMANTICS_H_
#define _SEMANTICS_H_
#include <string.h>
#include "treeNodes.h"
#include "treeUtils.h"
#include "symbolTable.h"

TreeNode *semanticAnalysis(TreeNode *syntree,          // pass in and return an annotated syntax tree
                           SymbolTable *symtabX,       // pass in and return the symbol table
                           int &globalOffset            // return the offset past the globals
    );
#endif
