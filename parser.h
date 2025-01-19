#ifndef PARSER_H
#define PARSER_H

#include "lex.h"

// AST Node definition
typedef struct Node {
    char *value;
    struct Node **children;
    int childCount;
} Node;


Node *createNode(const char *value);
void addChild(Node *parent, Node *child);
void printAST(Node *node, int depth);
void freeAST(Node *node);
int errorCheck(TokenType expected, const char *message);
void errorRecovery();
void errorMessage(const char *message);
void initOutputFile(const char *filename);


Node *parseProgram(Token *tokens);
Node *parseMainFuntion();
Node *parseDeclarations();
Node *parseDeclarationStmt();
Node *parseFuncDeclaration();
Node *parseStmt();
Node *parseExpression();
Node *parseLogicalExpr();
Node *parseRelationalExpr();
Node *parseArithmeticExpr();
Node *parseTerm();
Node *parseFactor();
Node *parsePower();
Node *parseUnaryExpr();
Node *parseUnaryOperator();
Node *parsePrimaryExpr();
Node *parseAssignStmt();
Node *parseCondStmt();
Node *parseIterStmt();
Node *parseInputStmt();
Node *parseOutputStmt();
Node *parseReturnStmt();
Node *parseType();
Node *parseValue();
Node *parseArrayDeclaration();
Node *parseArgument();
Node *parseIfStmt();
Node *parsElseIfStmt();
Node *parseElseStmt();
Node *parseSwitchStmt();
Node *parseCaseClause();

#endif
