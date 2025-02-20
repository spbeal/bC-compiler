#ifndef _UTIL_H_
#define _UTIL_H_
#include "treeNodes.h"
#include "scanType.h"
#include <string.h>

#include <cstdio>
#include <iostream>
#include <unistd.h>

// lots of these save the TokenData block so line number and yytext are saved
TreeNode *newDeclNode(DeclKind kind,
                      ExpType type,
                      TokenData *token=NULL,
                      TreeNode *c0=NULL,
                      TreeNode *c1=NULL,
                      TreeNode *c2=NULL);  // save TokenData block!!
TreeNode *newStmtNode(StmtKind kind,
                      TokenData *token,
                      TreeNode *c0=NULL,
                      TreeNode *c1=NULL,
                      TreeNode *c2=NULL);
TreeNode *newExpNode(ExpKind kind,
                     TokenData *token,
                     TreeNode *c0=NULL,
                     TreeNode *c1=NULL,
                     TreeNode *c2=NULL);
char *tokenToStr(int type);
char *expTypeToStr(ExpType type, bool isArray=false, bool isStatic=false);
void printTreeNode(FILE *out, TreeNode *syntaxTree);
void printTree(FILE *out, TreeNode *syntaxTree);
void printTreeRec(FILE *listing, int depth, int siblingCnt, TreeNode *tree);
char* type_str(ExpType type, bool isStatic, bool isArray);

TreeNode *addSibling(TreeNode *t, TreeNode *s);
void setType(TreeNode *t, ExpType type, bool isStatic);

void initTokenStrings();
TreeNode* initializeNode(TreeNode* c0, TreeNode* c1, TreeNode* c2, TokenData *token);
#endif
