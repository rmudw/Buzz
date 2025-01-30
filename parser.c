#include "lex.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

Token *token;
int currentToken = 0;
int hasError = 0;
int successParse = 1;

FILE *outputFile;

void initOutputFile(const char *filename) {
    outputFile = fopen(filename, "w");
    if (!outputFile) {
        fprintf(stderr, "Error: Could not open output file %s\n", filename);
        exit(EXIT_FAILURE);
    }
}

void closeOutputFile() {
    if (outputFile) {
        fclose(outputFile);
    }
}

// returns next token
Token *peekToken() {
    return &token[currentToken];
}

void advanceToken() {
    if (token[currentToken].type != END_OF_TOKENS) {
        currentToken++;
    }
}

Node *createNode(const char *value) {
    Node *node = malloc(sizeof(Node));
    node->value = strdup(value);
    node->children = NULL;
    node->childCount = 0;
    return node;
}

void addChild(Node *parent, Node *child) {
    parent->children = realloc(parent->children, sizeof(Node *) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}

void printAST(Node *node, int depth) {
    /*for (int i = 0; i < depth; i++) {
        printf("  ");
    }*/
    //printf("%s\n", node->value);

    if (outputFile) {
        for (int i = 0; i < depth; i++) fprintf(outputFile, "  ");
        fprintf(outputFile, "%s\n", node->value);
    }
    
    for (int i = 0; i < node->childCount; i++) {
        printAST(node->children[i], depth + 1);
    }
}

Node *parseProgram(Token *tokens) { //buzz beegin {<declaration>} <mf> beegone
    initOutputFile("outputParser.txt");
    token = tokens;
    Node *node = createNode("<program>");

    if(errorCheck(BUZZ_TOKEN, "Expected 'buzz' at the beginning of the program") == false) {
        addChild(node, createNode("buzz"));
        advanceToken();
    } else {
        return node;
    }
        
    if(errorCheck(BEEGIN_TOKEN, "Expected 'beegin' after 'buzz'") == false) {
        addChild(node, createNode("beegin"));
        advanceToken();
    } else {
        return node;
    }

    while (peekToken()->type != QUEENBEE_TOKEN && peekToken()->type != END_OF_TOKENS) {
        addChild(node, parseDeclaration()); // parse any global declarations/functions
        if(peekToken()->type != QUEENBEE_TOKEN) {
            advanceToken();
        }
    }

    if(peekToken()->type == QUEENBEE_TOKEN) {
        addChild(node, parseMainFuntion());
    } else {
        errorMessage("'queenbee' was not found");
        hasError = 1;
        successParse = 0;
        errorRecovery();
        addChild(node, createNode("Error"));
    }

    if(peekToken()->type == BEEGONE_TOKEN) {
        addChild(node, createNode("beegone"));
        advanceToken();
    } else {
        errorMessage("Expected 'beegone' at the end of the program");
        hasError = 1;
        successParse = 0;
        errorRecovery();
        addChild(node, createNode("Error"));
    }
    
    if(successParse == 1) {
        printf("\n%s", "Program successfully parsed.");
    }

    //printf("\n\n");
    printAST(node, 0);
    return node;
}

Node *parseDeclaration() {
    Node *node = createNode("<declaration>");

    if (peekToken()->type == INT_TOKEN || peekToken()->type == CHAR_TOKEN ||
        peekToken()->type == FLOAT_TOKEN || peekToken()->type == CHAIN_TOKEN ||
        peekToken()->type == BOOL_TOKEN) {
        advanceToken();

        if(peekToken()->type == VAR_IDENT) {
            currentToken--;
            addChild(node, parseDeclarationStmt());

            if (errorCheck(SEMICOLON, "Expected ';' after declaration statement") != 0) {
                errorRecovery();
                hasError = 1;
                successParse = 0;
                addChild(node, createNode("Error"));
            }
        } else {
            currentToken--;
            addChild(node, parseType());
            
            errorMessage("Expected identifier after type");
            errorRecovery();
            hasError = 1;
            successParse = 0;
            addChild(node, createNode("Error"));
            return node;
        }
    } else if(peekToken()->type == VAR_IDENT) {
        parseAssignStmt();
    } else {
        return createNode("Error");
    }

    return node;
}

Node *parseType() {
    Node *node = createNode("<type>");
    if (peekToken()->type == INT_TOKEN || peekToken()->type == FLOAT_TOKEN || peekToken()->type == BOOL_TOKEN
        || peekToken()->type == CHAR_TOKEN || peekToken()->type == CHAIN_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        return node;
    }

    errorMessage("Expected a valid type token");
    errorRecovery();
    hasError = 1;
    addChild(node, createNode("Error"));
    return node; // Return an error node in case of failure
}

Node *parseDeclarationStmt() {
    Node *node = createNode("<declaration_stmt>");
    addChild(node, parseType()); 
    advanceToken();

    if (peekToken()->type == VAR_IDENT) {
        do {
            if (token[currentToken + 1].type == ASSIGNMENT_OP) {
                addChild(node, parseAssignStmt());
            } else {
                addChild(node, createNode(peekToken()->value));
                advanceToken(); 
            }

            if (peekToken()->type == COMMA) {
                advanceToken(); 
            } else {
                break; 
            }
        } while (peekToken()->type == VAR_IDENT);
    } else {
        errorMessage("Expected a variable identifier after type");
        errorRecovery();
        hasError = 1;
        successParse = 0;
        addChild(node, createNode("Error"));
        return node;
    }

    return node;
}

Node *parseAssignStmt() {
    Node *node = createNode("<assign_stmt>");

    if (peekToken()->type == VAR_IDENT) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();

        if (errorCheck(ASSIGNMENT_OP, "Expected '=' in assignment statement") == false) {
            addChild(node, createNode(peekToken()->value));
            advanceToken();
            if(peekToken()->type == DBL_QUOTE || peekToken()->type == SNGL_QUOTE) {
                advanceToken();
            }

            addChild(node, parseExpression());
            if(peekToken()->type == DBL_QUOTE || peekToken()->type == SNGL_QUOTE) {
                advanceToken();
            }
        }
    } else {
        errorMessage("Expected variable in assignment statement");
        errorRecovery();
        hasError = 1;
        successParse = 0;
        addChild(node, createNode("Error"));
    }

    return node;
}

Node *parseExpression() {
    Node *node = createNode("<expression>");
    addChild(node, parseLogicalExpr());

    return node;
}

Node *parseLogicalExpr() {
    Node *node = createNode("<logical_expr>");
    
    Node *child = parseRelationalExpr();
 

    while (peekToken()->type == AND || peekToken()->type == OR) {
        Node *operatorNode = createNode(peekToken()->value);
        advanceToken();
        addChild(operatorNode, child);
        addChild(operatorNode, parseRelationalExpr());
        child = operatorNode;
    }

    addChild(node, child);
    return node;
}

Node *parseRelationalExpr() {
    Node *node = createNode("<relational_expr>");

    Node *child = parseArithmeticExpr();


    while (peekToken()->type == IS_EQUAL_TO || peekToken()->type == NOT_EQUAL ||
           peekToken()->type == LESS_THAN || peekToken()->type == GREATER_THAN ||
           peekToken()->type == LESS_EQUAL || peekToken()->type == GREATER_EQUAL) {
        Node *operatorNode = createNode(peekToken()->value);
        advanceToken();
        addChild(operatorNode, child);
        addChild(operatorNode, parseArithmeticExpr());
        child = operatorNode;
    }

    addChild(node, child);
    return node;
}

Node *parseArithmeticExpr() {
    Node *node = createNode("<arithmetic_expr>");

    Node *child = parseTerm();
 

    while (peekToken()->type == ADDITION || peekToken()->type == SUBTRACTION) {
        Node *operatorNode = createNode(peekToken()->value);
        advanceToken();
        addChild(operatorNode, child);
        addChild(operatorNode, parseTerm());
        child = operatorNode;
    }

    addChild(node, child);
    return node;
}

Node *parseTerm() {
    Node *node = createNode("<term>");

    Node *child = parseFactor();

    while (peekToken()->type == MULTIPLICATION || peekToken()->type == DIVISION ||
           peekToken()->type == MODULO || peekToken()->type == INT_DIVISION) {
        Node *operatorNode = createNode(peekToken()->value);
        advanceToken();
        addChild(operatorNode, child);
        addChild(operatorNode, parseFactor());
        child = operatorNode;
    }

    addChild(node, child);
    return node;
}

Node *parseFactor() {
    Node *node = createNode("<factor>");
    Node *child = parsePower();

    while (peekToken()->type == EXPONENT) {
        Node *operatorNode = createNode(peekToken()->value);
        advanceToken();
        addChild(operatorNode, child);
        addChild(operatorNode, parsePower());
        child = operatorNode;
    }

    addChild(node, child);
    return node;
}

Node *parsePower() {
    Node *node = createNode("<power>");

    if (peekToken()->type == LEFT_PAREN) {
        advanceToken();
        addChild(node, parseExpression());
        if(errorCheck(RIGHT_PAREN, "Expected ')'") == false) {
            return node;
        } else {
            errorRecovery();
            hasError = 1;
            successParse = 0;
            addChild(node, createNode("Error"));
        }
    } else {
        addChild(node, parseUnaryExpr());
    }

    return node;
}

Node *parseUnaryExpr() {
     Node *node = createNode("<unary_expr>");
    
    if (peekToken()->type == INCREMENT || peekToken()->type == DECREMENT) {
        Node *nodeUnary = createNode(peekToken()->value);
        advanceToken();
        addChild(node, nodeUnary);
        addChild(nodeUnary, parsePrimaryExpr());
        return node;
    }
    
    Node *nodePrimary = parsePrimaryExpr();
    addChild(node, nodePrimary);
    
    if (peekToken()->type == INCREMENT || peekToken()->type == DECREMENT) {
        Node *operatorNode = createNode(peekToken()->value);
        advanceToken();
        addChild(node, operatorNode);
    }
    return node;
}

Node *parsePrimaryExpr() {
    Node *node = createNode("<primary_expr>");

    if (peekToken()->type == VAR_IDENT) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();
    } else if(peekToken()->type == INTEGER || peekToken()->type == FLOAT
            || peekToken()->type == STRING || peekToken()->type == CHARACTER
            || peekToken()->type == TRUE_TOKEN || peekToken()->type == FALSE_TOKEN) {
        addChild(node, parseValue());
        advanceToken();
    } else {
        errorMessage("Expected variable or literal in expression");

        addChild(node, createNode("Error"));
    }

    return node;
}

Node *parseValue() {
    Node *node = createNode("<value>");

    if(peekToken()->type == INTEGER || peekToken()->type == FLOAT || peekToken()->type == STRING
        || peekToken()->type == CHARACTER || peekToken()->type == FALSE_TOKEN || peekToken()->type == TRUE_TOKEN) {
        addChild(node, createNode(peekToken()->value));
    }

    return node;
}

Node *parseMainFuntion() {
    Node *node = createNode("<mf>");
    addChild(node, createNode("queenbee"));
    advanceToken();

    if(peekToken()->type == LEFT_BRACE) {
        advanceToken();

        while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
            addChild(node, parseStmt());
        }
       
        if(peekToken()->type == RIGHT_BRACE) {
            advanceToken();
            return node;
        } else {
                errorMessage("Expected '}' to close 'queenbee' block");
                errorRecovery();
                hasError = 1;
                successParse = 0;
                addChild(node, createNode("Error"));
            }
    } else {
        errorMessage("Expected '{' after 'queenbee'");
        errorRecovery();
        hasError = 1;
        successParse = 0;
        addChild(node, createNode("Error"));
    }

   
    return node;
}

Node *parseStmt() {
    Node *node = createNode("<stmt>");

    if (peekToken()->type == GATHER_TOKEN) {
        addChild(node, parseInputStmt());
    } else if (peekToken()->type == VAR_IDENT) {
        Node *assignNode = parseAssignStmt();

        if (peekToken()->type == SEMICOLON) {
            advanceToken();
            addChild(node, assignNode);
            return node;
        } else {
            errorMessage("Expected ';' after assignment statement");
            errorRecovery();
            hasError = 1;
            successParse = 0;
            addChild(node, createNode("Error"));
            return node;
        }
    } else if (peekToken()->type == BUZZOUT_TOKEN) {
        addChild(node, parseOutputStmt());
    } else if (peekToken()->type == IF_TOKEN || peekToken()->type == SWITCH_TOKEN) {
        addChild(node, parseCondStmt());
    } else if (peekToken()->type == WHILE_TOKEN || peekToken()->type == FOR_TOKEN || peekToken()->type == DO_TOKEN) {
       addChild(node, parseIterStmt());
    } else if (peekToken()->type == INT_TOKEN || peekToken()->type == FLOAT_TOKEN || peekToken()->type == CHAR_TOKEN
             || peekToken()->type == CHAIN_TOKEN  || peekToken()->type == BOOL_TOKEN) {
        Node *declarNode = parseDeclarationStmt();
        
        if (peekToken()->type == SEMICOLON) {
            advanceToken();
            addChild(node, declarNode);
            return node;
        } else {
            errorMessage("Expected ';' after declaration statement");
            errorRecovery();
            hasError = 1;
            successParse = 0;
            addChild(node, createNode("Error"));
            return node;
        }
    } else if (peekToken()->type == RETURN_TOKEN) {
        addChild(node, parseReturnStmt());
    } else {
        errorMessage("Invalid statement");
        errorRecovery();
        hasError = 1;
        successParse = 0;
        addChild(node, createNode("Error"));
    }

    return node;
}

Node *parseInputStmt() {
    Node *node = createNode("<input_stmt>");

    if (peekToken()->type == GATHER_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();

        if(peekToken()->type == VAR_IDENT) {
            addChild(node, createNode(peekToken()->value));
            advanceToken();

            if(peekToken()->type != SEMICOLON) {
                errorMessage("Expected ';' at the end of the input statement");
                errorRecovery();
                hasError = 1;
                successParse = 0;
                addChild(node, createNode("Error"));
            } else {
                advanceToken();
                return node;
            }
        } else {
            errorMessage("Expected variable identifier after 'gather'");
            errorRecovery();
            hasError = 1;
            successParse = 0;
            addChild(node, createNode("Error"));
        }
    }

    return node;
}

Node *parseCondStmt() {
    Node *node = createNode("<cond_stmt>");

    if(peekToken()->type == IF_TOKEN) {
        addChild(node, parseIfStmt());
    } else if(peekToken()->type == SWITCH_TOKEN) {
        addChild(node, parseSwitchStmt());
    }

    return node;
}

Node *parseIfStmt() {
    Node *node = createNode("<if_stmt>");

    if(peekToken()->type == IF_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();
        if(peekToken()->type == LEFT_PAREN) {
            advanceToken();
            Node *condition = parseExpression();
            if (condition) {
                addChild(node, condition);
                if(peekToken()->type == RIGHT_PAREN) {
                    advanceToken();
                    if(peekToken()->type == LEFT_BRACE) {
                        advanceToken();
                        while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
                            Node *stmt = parseStmt();
                            addChild(node, stmt);
                            //advanceToken();
                        }
                         
                        if(peekToken()->type == RIGHT_BRACE) {
                            advanceToken();
                            if(peekToken()->type == ELSEIF_TOKEN) { 
                                while(peekToken()->type == ELSEIF_TOKEN  && peekToken()->type != END_OF_TOKENS) {
                                    addChild(node, parseElseIfStmt());
                                    //advanceToken();
                                }
                            } 
                            
                            if(peekToken()->type == ELSE_TOKEN) {
                                addChild(node, parseElseStmt());
                                //advanceToken();
                            } else {
                                return node;
                            }
                        } else {
                            errorMessage("Expected a '}' after 'if' block");
                            addChild(node, createNode("Error"));
                            errorRecovery();
                            hasError = 1;     
                            successParse = 0;  
                        }
                    } else {
                        errorMessage("Expected a '{' after condition");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;     
                        successParse = 0;  
                    }
                } else {
                    errorMessage("Expected a ')' after expression");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                    successParse = 0;
                }
            } else {
                errorMessage("Expected an expression inside 'if' condition");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;
                successParse = 0;
            }
        } else {
            errorMessage("Expected a '(' after 'if'");
            errorRecovery();
            hasError = 1;
            successParse = 0;
            addChild(node, createNode("Error"));
        }
    }
    return node;
}

Node *parseElseIfStmt() {
    Node *node = createNode("<elseif_stmt>");

    if(peekToken()->type == ELSEIF_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();

        if(peekToken()->type == LEFT_PAREN) {
            advanceToken();
            Node *condition = parseExpression();
            if (condition) {
                    addChild(node, condition);
                    if(peekToken()->type == RIGHT_PAREN) {
                        advanceToken();
                        if(peekToken()->type == LEFT_BRACE) {
                            advanceToken();
                            while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
                                Node *stmt = createNode("<stmt>");
                                addChild(node, stmt);
                                addChild(stmt, parseStmt());
                                //advanceToken();
                            }

                            if(peekToken()->type == RIGHT_BRACE) {
                                advanceToken();
                                return node;
                            } else {
                                errorMessage("Expected a '}' to close 'elseif' block");
                                addChild(node, createNode("Error"));
                                errorRecovery();
                                hasError = 1;  
                                successParse = 0;
                            }
                        } else {
                            errorMessage("Expected a '{' after condition");
                            addChild(node, createNode("Error"));
                            errorRecovery();
                            hasError = 1;       
                            successParse = 0;
                        }
                    } else {
                        errorMessage("Expected a ')' after expression");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;
                        successParse = 0;
                    }
                } else {
                    errorMessage("Expected an expression inside 'elseif' condition");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                    successParse = 0;
                }
        } else {
            errorMessage("Expected a '(' after 'elseif'");
            errorRecovery();
            hasError = 1;
            successParse = 0;
            addChild(node, createNode("Error"));
        }
    }

    return node;
}

Node *parseElseStmt() { 
    Node *node = createNode("<else_stmt>");

    if(peekToken()->type == ELSE_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();

        if(peekToken()->type == LEFT_BRACE) {
            advanceToken();
            while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
                Node *stmt = createNode("<stmt>");
                addChild(node, stmt);
                addChild(stmt, parseStmt());
                //advanceToken();
            }
            if(peekToken()->type == RIGHT_BRACE) {
                advanceToken();
                return node;
            } else {
                errorMessage("Expected a '}' to close 'else' block");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;  
            }
        } else {
            errorMessage("Expected a '{' after 'else'");
            addChild(node, createNode("Error"));
            errorRecovery();
            hasError = 1;       
        }
    }

    return node;
}

Node *parseSwitchStmt() {
    Node *node = createNode("<switch_stmt>");

    if(peekToken()->type == SWITCH_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();

        if(peekToken()->type == LEFT_PAREN) {
            advanceToken();
            if (peekToken()->type == SNGL_QUOTE) { 
                advanceToken();
                if (peekToken()->type == CHARACTER) {
                    addChild(node, createNode(peekToken()->value));
                    advanceToken();
                    if (peekToken()->type == SNGL_QUOTE) {
                        advanceToken();
                    } else {
                        errorMessage("Expected closing single quote after character");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;
                        successParse = 0;
                        return node;
                    }
                } else {
                    errorMessage("Expected character inside single quote");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                    successParse = 0;
                    return node;
                }
            } else if (peekToken()->type == INTEGER || peekToken()->type == VAR_IDENT) {
                addChild(node, createNode(peekToken()->value));
                advanceToken();
            } else {
                errorMessage("Switch condition must be an integer or character in single quotes");
                addChild(node, createNode("Error"));
                hasError = 1;
                successParse = 0;
                return node;
            }

            if(peekToken()->type == RIGHT_PAREN) {
                advanceToken();
                if(peekToken()->type == LEFT_BRACE) {
                    advanceToken();
                    while(peekToken()->type == CASE_TOKEN) {
                        addChild(node, parseCaseClause());
                    } 
                    
                    if(peekToken()->type == DEFAULT_TOKEN) {
                        Node *defaultNode = createNode("default");
                        addChild(node, defaultNode);
                        advanceToken();

                        if(peekToken()->type == COLON) {
                            advanceToken();

                            while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
                                if(peekToken()->type == VAR_IDENT || peekToken()->type == INTEGER || peekToken()->type == FLOAT) {
                                    advanceToken();
                                    if(peekToken()->type != ASSIGNMENT_OP) {
                                        currentToken--;
                                        Node *expr = createNode("<expression>");
                                        addChild(node, expr);
                                        addChild(expr, parseExpression());

                                        if(peekToken()->type == SEMICOLON) {
                                            advanceToken();
                                            if(peekToken()->type == RIGHT_BRACE) {
                                                break;
                                            } else {
                                                continue;
                                            }
                                        } else {
                                            errorMessage("Expected ';' after expression");
                                            hasError = 1;
                                            successParse = 0;
                                            errorRecovery();
                                            addChild(node, createNode("Error"));
                                        }
                                    } else {
                                        currentToken--;
                                    }
                                }
                                
                                // Node *stmt = createNode("<stmt>");
                                //addChild(node, stmt);
                                addChild(node, parseStmt());
                            }

                            if(peekToken()->type == RIGHT_BRACE) {
                                advanceToken();
                                return node;
                            } else {
                                errorMessage("Expected '}' to close switch block");
                                addChild(node, createNode("Error"));
                                errorRecovery();
                                hasError = 1;
                                successParse = 0; 
                            }
                        } else {
                            errorMessage("Expected ':' after 'default'");
                            addChild(node, createNode("Error"));
                            errorRecovery();
                            hasError = 1;
                            successParse = 0;
                        }
                    } else {
                        errorMessage("Expected 'default");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;
                        successParse = 0;
                    }
                } else {
                    errorMessage("Expected a '{' after condition");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;       
                    successParse = 0;
                }
            } else {
                errorMessage("Expected ')' after switch condition");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;
                successParse = 0;
            }
        } else {
                errorMessage("Expected '(' after 'switch");
                hasError = 1;
                successParse = 0;
                errorRecovery();
                addChild(node, createNode("Error"));
        }
    }
    return node;
}

Node *parseCaseClause() {
    Node *node = createNode("<case_clause>");

    if(peekToken()->type == CASE_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();

        if (peekToken()->type == SNGL_QUOTE) { 
            advanceToken();
            if (peekToken()->type == CHARACTER) {
                addChild(node, createNode(peekToken()->value));
                advanceToken();
                if (peekToken()->type == SNGL_QUOTE) {
                    advanceToken();
                } else {
                    errorMessage("Expected closing single quote after character");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                    successParse = 0;
                    return node;
                }
            } else {
                errorMessage("Expected character inside single quote");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;
                successParse = 0;
                return node;
            }
            
        } else if (peekToken()->type == INTEGER) {
                addChild(node, createNode(peekToken()->value));
                advanceToken();
        } else {
                errorMessage("Case condition must be an integer or character");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;
                successParse = 0;
                return node;
        }

        if(peekToken()->type == COLON) {
            advanceToken();
            
            while (peekToken()->type != STING_TOKEN && peekToken()->type != CASE_TOKEN && peekToken()->type != DEFAULT_TOKEN && peekToken()->type != END_OF_TOKENS) {
                if(peekToken()->type == VAR_IDENT || peekToken()->type == INTEGER || peekToken()->type == FLOAT) {
                    advanceToken();
                    if(peekToken()->type != ASSIGNMENT_OP) {
                        currentToken--;
                        Node *expr = createNode("<expression>");
                        addChild(node, expr);
                        addChild(expr, parseExpression());

                        if(peekToken()->type == SEMICOLON) {
                            advanceToken();
                            if(peekToken()->type == RIGHT_BRACE) {
                                break;
                            } else {
                                continue;
                            }
                        } else {
                            errorMessage("Expected ';' after expression");
                            hasError = 1;
                            successParse = 0;
                            errorRecovery();
                            addChild(node, createNode("Error"));
                        }
                    } else {
                        currentToken--;
                    }
                }
                
               // Node *stmt = createNode("<stmt>");
                //addChild(node, stmt);
                addChild(node, parseStmt());
            }

            if(peekToken()->type == CASE_TOKEN || peekToken()->type == DEFAULT_TOKEN) {
                errorMessage("Expected 'sting' keyword to end case");
                hasError = 1;
                errorRecovery();
                addChild(node, createNode("Error"));
            } else if(peekToken()->type == STING_TOKEN) {
                addChild(node, createNode(peekToken()->value));
                advanceToken();
                if(peekToken()->type == SEMICOLON) {
                    advanceToken();
                    return node;
                } else {
                    errorMessage("Expected ';' after 'sting'");
                    hasError = 1;
                    errorRecovery();
                    addChild(node, createNode("Error"));
                }
            }
        } else {
            errorMessage("Expected ':' after case value");
            hasError = 1;
            errorRecovery();
            addChild(node, createNode("Error"));
        }
    } else {
        errorMessage("Expected constant after 'case'");
        hasError = 1;
        errorRecovery();
        addChild(node, createNode("Error"));
    }

    return node;
}

Node *parseOutputStmt() {
    Node *node = createNode("<output_stmt>");

    if (peekToken()->type == BUZZOUT_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();

        if(peekToken()->type == LEFT_PAREN) {
            advanceToken();

            if(peekToken()->type == DBL_QUOTE) {
                advanceToken();
                char *string = NULL;
                if(peekToken()->type == STRING) {
                    advanceToken();
                    if(peekToken()->type == DBL_QUOTE) {
                        currentToken--;
                        string = malloc(strlen(peekToken()->value) + 3);
                        snprintf(string, strlen(peekToken()->value) + 3, "\"%s\"", peekToken()->value);
                        advanceToken();
                    } else {
                        errorMessage("Missing closing quote");
                        hasError = 1;
                        successParse = 0;
                        errorRecovery();
                        addChild(node, createNode("Error"));
                    }
                } else if(peekToken()->type == DBL_QUOTE) {
                    string = strdup("\"\"");
                } else {
                    errorMessage("Missing closing quote");
                    hasError = 1;
                    successParse = 0;
                    errorRecovery();
                    addChild(node, createNode("Error"));
                }

                advanceToken();
                if (string) {
                    addChild(node, createNode(string)); // Add the string or empty string
                    free(string);
                }
        
                while (peekToken()->type == COMMA  && peekToken()->type != END_OF_TOKENS) {
                    advanceToken();
                    Node *argNode = parseArgument();
                    if (argNode) {
                        addChild(node, argNode);
                    } else {
                        errorMessage("Expected argument after ','");
                        errorRecovery();
                        addChild(node, createNode("Error"));
                        break;
                    }
                }
                
                if (errorCheck(RIGHT_PAREN, "Expected ')' to close argument list") == false) {
                    advanceToken();
                    if (errorCheck(SEMICOLON, "Expected ';' after output statement") == false) {
                        advanceToken();
                        return node;
                    } else {
                        hasError = 1;
                        successParse = 0;
                        errorRecovery();
                        addChild(node, createNode("Error"));
                    }
                } else {
                    hasError = 1;
                    successParse = 0;
                    errorRecovery();
                    addChild(node, createNode("Error"));
                }
            } else {
                errorMessage("Expected a string after '('");
                hasError = 1;
                successParse = 0;
                errorRecovery();
                addChild(node, createNode("Error"));
            }
        } else {
            errorMessage("Expected '(' after 'buzzout'");
            hasError = 1;
            successParse = 0;
            errorRecovery();
            addChild(node, createNode("Error"));
        }
    }

    return node;
}

Node *parseArgument() {
    Node *node = createNode("<argument>");

    if (peekToken()->type == VAR_IDENT || peekToken()->type == INTEGER || peekToken()->type == FLOAT ||
        peekToken()->type == STRING || peekToken()->type == CHARACTER) {
        addChild(node, createNode(peekToken()->value)); // Add the argument
        advanceToken(); // Consume the argument token
    } else {
        errorMessage("Expected a valid argument (variable or value)");
        errorRecovery();
        hasError = 1;
        successParse = 0;
        addChild(node, createNode("Error"));
    }

    return node;
}

Node *parseIterStmt() {
    Node *node =  createNode("<iter_stmt>");

    if(peekToken()->type ==  WHILE_TOKEN) {
        addChild(node, parseWhile());
    } else if(peekToken()->type == DO_TOKEN) {
        addChild(node, parseDoWhile());
    } else if(peekToken()->type == FOR_TOKEN) {
        addChild(node, parseForLoop());
    } else {
        errorMessage("Invalid iteration statement");
        errorRecovery();
        hasError = 1;
        addChild(node, createNode("Error"));
    }

    return node;
}

Node *parseWhile() {
    Node *node =  createNode(peekToken()->value);
    advanceToken();

    if(peekToken()->type == LEFT_PAREN) {
        advanceToken();
        Node *condition = parseExpression();
        if (condition) {
            addChild(node, condition);
            if(peekToken()->type == RIGHT_PAREN) {
                advanceToken();
                if(peekToken()->type == LEFT_BRACE) {
                    advanceToken();
                    while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
                        if(peekToken()->type == VAR_IDENT || peekToken()->type == INTEGER || peekToken()->type == FLOAT) {
                            advanceToken();
                            if(peekToken()->type != ASSIGNMENT_OP) {
                                currentToken--;
                                Node *expr = createNode("<expression>");
                                addChild(node, expr);
                                addChild(expr, parseExpression());

                                if(peekToken()->type == SEMICOLON) {
                                    advanceToken();
                                    if(peekToken()->type == RIGHT_BRACE) {
                                        break;
                                    } else {
                                        continue;
                                    }
                                } else {
                                    errorMessage("Expected ';' after expression");
                                    hasError = 1;
                                    successParse = 0;
                                    errorRecovery();
                                    addChild(node, createNode("Error"));
                                }
                            } else {
                                currentToken--;
                            }
                        }
                        
                        Node *stmt = createNode("<stmt>");
                        addChild(node, stmt);
                        addChild(stmt, parseStmt());

                    }
                    

                    if(peekToken()->type == RIGHT_BRACE) {
                        advanceToken();
                        return node;
                    } else {
                        errorMessage("Expected '}' to close while loop");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;
                        successParse = 0;
                    }
                } else {
                    errorMessage("Expected a '{' after while condition");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                    successParse = 0;
                }
            } else {
                errorMessage("Expected a ')' after expression");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;
                successParse = 0;
            }

        } else {
            errorMessage("Expected an expression inside 'while' condition");
            addChild(node, createNode("Error"));
            errorRecovery();
            hasError = 1;
            successParse = 0;
        }
    } else {
        errorMessage("Expected '(' after 'while'");
        hasError = 1;
        successParse = 0;
        errorRecovery();
        addChild(node, createNode("Error"));
    }

    return node;
}

Node *parseDoWhile() {
    Node *node =  createNode(peekToken()->value);
    advanceToken();

    if(peekToken()->type == LEFT_BRACE) {
        advanceToken();
        while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
            if(peekToken()->type == VAR_IDENT || peekToken()->type == INTEGER || peekToken()->type == FLOAT) {
                advanceToken();
                if(peekToken()->type != ASSIGNMENT_OP) {
                    currentToken--;
                    Node *expr = createNode("<expression>");
                    addChild(node, expr);
                    addChild(expr, parseExpression());

                    if(peekToken()->type == SEMICOLON) {
                        advanceToken();
                        if(peekToken()->type == RIGHT_BRACE) {
                            break;
                        } else {
                            continue;
                        }
                    } else {
                        errorMessage("Expected ';' after expression");
                        hasError = 1;
                        successParse = 0;
                        errorRecovery();
                        addChild(node, createNode("Error"));
                    }
                } else {
                    currentToken--;
                }
            }
            
            Node *stmt = createNode("<stmt>");
            addChild(node, stmt);
            addChild(stmt, parseStmt());
        }

        if(peekToken()->type == RIGHT_BRACE) {
            advanceToken();
            if(peekToken()->type == WHILE_TOKEN) {
                addChild(node, createNode(peekToken()->value));
                advanceToken();
                if(peekToken()->type == LEFT_PAREN) {
                    advanceToken();

                    Node *condition = parseExpression();
                    if (condition) {
                        addChild(node, condition);
                        if(peekToken()->type == RIGHT_PAREN) {
                            advanceToken();
                            if(peekToken()->type == SEMICOLON) {
                                advanceToken();
                                return node;
                            } else {
                                errorMessage("Expected ';' after 'while'");
                                hasError = 1;
                                successParse = 0;
                                errorRecovery();
                                addChild(node, createNode("Error"));
                            }
                        } else {
                            errorMessage("Expected ')' after expression");
                            hasError = 1;
                            successParse = 0;
                            errorRecovery();
                            addChild(node, createNode("Error"));
                        }
                    } else {
                        errorMessage("Expected an expression inside 'while' condition");
                        hasError = 1;
                        successParse = 0;
                        errorRecovery();
                        addChild(node, createNode("Error"));
                    }
                } else {
                    errorMessage("Expected '(' after 'while'");
                    hasError = 1;
                    successParse = 0;
                    errorRecovery();
                    addChild(node, createNode("Error"));
                }
            } else {
                errorMessage("Expected 'while' after 'do' block");
                hasError = 1;
                successParse = 0;
                errorRecovery();
                addChild(node, createNode("Error"));
            }
        } else {
            errorMessage("Expected '}' to close 'do-while' body");
            hasError = 1;
            successParse = 0;
            errorRecovery();
            addChild(node, createNode("Error"));
        }
    } else {
        errorMessage("Expected '{' after 'do'");
        hasError = 1;
        successParse = 0;
        errorRecovery();
        addChild(node, createNode("Error"));
    }

    return node;
}

Node *parseForLoop() {
    Node *node = createNode("<for_stmt>");

    if (peekToken()->type == FOR_TOKEN) {
        addChild(node, createNode(peekToken()->value));
        advanceToken();

        if (peekToken()->type == LEFT_PAREN) {
            advanceToken();

            // Parse initialization
            Node *init = parseAssignStmt();
            if (init) {
                addChild(node, init);
                //advanceToken();
                if (peekToken()->type == SEMICOLON) {
                    advanceToken();
                    Node *condition = parseExpression();
                    if (condition) {
                        addChild(node, condition);
                        //advanceToken();
                        if (peekToken()->type == SEMICOLON) {
                            advanceToken();
                            Node *iteration;
                            if(token[currentToken + 1].type == ASSIGNMENT_OP) {
                                iteration = parseAssignStmt();
                            }
                            else if(token[currentToken + 1].type != ASSIGNMENT_OP) {
                                iteration = parseExpression();
                            }

                            if (iteration) {
                                addChild(node, iteration);
                                if (peekToken()->type == RIGHT_PAREN) {
                                    advanceToken();
                                   if (peekToken()->type == LEFT_BRACE) {
                                        advanceToken();
                                        while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
                                            if(peekToken()->type == VAR_IDENT || peekToken()->type == INTEGER || peekToken()->type == FLOAT) {
                                                advanceToken();
                                                if(peekToken()->type != ASSIGNMENT_OP) {
                                                    currentToken--;
                                                    Node *expr = createNode("<expression>");
                                                    addChild(node, expr);
                                                    addChild(expr, parseExpression());

                                                    if(peekToken()->type == SEMICOLON) {
                                                        advanceToken();
                                                        if(peekToken()->type == RIGHT_BRACE) {
                                                            break;
                                                        } else {
                                                            continue;
                                                        }
                                                    } else {
                                                        errorMessage("Expected ';' after expression");
                                                        hasError = 1;
                                                        successParse = 0;
                                                        errorRecovery();
                                                        addChild(node, createNode("Error"));
                                                    }
                                                } else {
                                                    currentToken--;
                                                }
                                            }
                                            
                                            Node *stmt = createNode("<stmt>");
                                            addChild(node, stmt);
                                            addChild(stmt, parseStmt());
                                        }

                                        if (peekToken()->type == RIGHT_BRACE) {
                                            advanceToken();
                                            return node;
                                        } else {
                                            errorMessage("Expected '}' to close block");
                                            errorRecovery();
                                            hasError = 1;
                                            addChild(node, createNode("Error"));
                                        }
                                    } else {
                                        errorMessage("Expected '{' to start block");
                                        errorRecovery();
                                        hasError = 1;
                                        addChild(node, createNode("Error"));
                                    }
                                } else {
                                    errorMessage("Expected ')' after 'for' loop components");
                                    addChild(node, createNode("Error"));
                                    errorRecovery();
                                    hasError = 1;
                                }
                            } else {
                                errorMessage("Expected iteration expression in 'for'");
                                addChild(node, createNode("Error"));
                                errorRecovery();
                                hasError = 1;
                            }
                        } else {
                            errorMessage("Expected ';' after condition in 'for'");
                            addChild(node, createNode("Error"));
                            errorRecovery();
                            hasError = 1;
                        }
                    } else {
                        errorMessage("Expected condition in 'for'");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;
                    }
                } else {
                    errorMessage("Expected ';' after initialization in 'for'");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                }
            } else {
                errorMessage("Expected initialization in 'for'");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;
            }
        } else {
            errorMessage("Expected '(' after 'for'");
            addChild(node, createNode("Error"));
            errorRecovery();
            hasError = 1;
        }
    }

    return node;
}

Node *parseReturnStmt() {
    Node *node = createNode("<return_stmt>");
    
    if (peekToken()->type == RETURN_TOKEN) {
        // Add 'returns' token to AST
        addChild(node, createNode(peekToken()->value));
        advanceToken();
        
        // Check if there's an expression after 'returns'
        if (peekToken()->type != SEMICOLON) {
            // Parse the expression
            Node *expr = parseExpression();
            if (expr) {
                addChild(node, expr);
                
                // Check for semicolon
                if (peekToken()->type == SEMICOLON) {
                    advanceToken();
                } else {
                    errorMessage("Expected ';' after return expression");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                    successParse = 0;
                }
            } else {
                errorMessage("Invalid expression in return statement");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;
                successParse = 0;
            }
        } else {
            // Handle case of 'returns;' with no expression
            advanceToken(); // consume semicolon
        }
    } else {
        errorMessage("Expected 'returns' keyword");
        addChild(node, createNode("Error"));
        errorRecovery();
        hasError = 1;
        successParse = 0;
    }
    
    return node;
}

int errorCheck(TokenType expected, const char *message) {
    if (peekToken()->type == expected) {
        return 0;
    } else {
        errorMessage(message);
        return 1;
    }
}

// panic error recory, wait for semicolon and right brace before resuming parsing
void errorRecovery() {

    while (peekToken()->type != SEMICOLON && peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS 
        && peekToken()->type != INT_TOKEN && peekToken()->type != FLOAT_TOKEN && peekToken()->type != CHAIN_TOKEN
        && peekToken()->type != CHAR_TOKEN && peekToken()->type != HIVE_TOKEN && peekToken()->type != BOOL_TOKEN
        && peekToken()->type != QUEENBEE_TOKEN) {
        advanceToken();
    }
    
    if(peekToken()->type == SEMICOLON) {
        advanceToken();
    }
}

void errorMessage(const char *message) {
    printf("Line %d   Error: %s, have '%s'\n", peekToken()->line, message, peekToken()->value);
   /* if (outputFile) {
        fprintf(outputFile, "Line %d   Error: %s, have '%s'\n", peekToken()->line, message, peekToken()->value);
    }*/
}