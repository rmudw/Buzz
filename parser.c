#include "lex.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

Token *token;
int currentToken = 0;
int hasError = 0;

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
    for (int i = 0; i < depth; i++) printf("  ");
    printf("%s\n", node->value);

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
            addChild(node, parseDeclarations()); // parse any global declarations/functions
            if(peekToken()->type != QUEENBEE_TOKEN) {
                advanceToken();
            }
        }
        
        if(errorCheck(QUEENBEE_TOKEN, "'queenbee' not found") == false) {
            addChild(node, parseMainFuntion());
            if(hasError == 1) {
                advanceToken();
            }
        } else {
            return node;
        }

        if(errorCheck(BEEGONE_TOKEN, "Expected 'beegone' at the end of the program") == false) {
            addChild(node, createNode("beegone"));
        }
    

    printf("\n\n");
    printAST(node, 0);
    return node;
}

Node *parseDeclarations() {
    Node *node = createNode("<declarations>");

    if (peekToken()->type == INT_TOKEN || peekToken()->type == CHAR_TOKEN ||
        peekToken()->type == FLOAT_TOKEN || peekToken()->type == CHAIN_TOKEN ||
        peekToken()->type == BOOL_TOKEN) {
        advanceToken();
        // check if data type is followed by an identifier
        if(peekToken()->type == VAR_IDENT) {
            currentToken--;
            addChild(node, parseDeclarationStmt());
        } else if(peekToken()->type == FUNC_IDENT) {
            currentToken--;
            //parseFuncDeclaration();
        } else {
            currentToken--;
            addChild(node, parseType());
            
            errorMessage("Expected identifier after type");
            errorRecovery();
            hasError = 1;
            addChild(node, createNode("Error"));
            return node;
        }
    } else if(peekToken()->type == VAR_IDENT) {
        parseAssignStmt();
    } else {
        //errorRecovery("Invalid declaration");
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
        addChild(node, createNode("Error"));
        return node;
    }

    if (errorCheck(SEMICOLON, "Expected ';' after declaration statement") != 0) {
        errorRecovery();
        hasError = 1;
        addChild(node, createNode("Error"));
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
        Node *operatorNode = createNode(peekToken()->value);
        advanceToken();
        addChild(operatorNode, parsePrimaryExpr());

        if (peekToken()->type == INCREMENT || peekToken()->type == DECREMENT) {
            addChild(operatorNode, createNode(peekToken()->value));
            advanceToken();
        }

        addChild(node, operatorNode);
    } else {
        addChild(node, parsePrimaryExpr());
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

    if(errorCheck(LEFT_BRACE, "Expected '{' after 'queenbee'") == false) {
        advanceToken();
        while (peekToken()->type != RIGHT_BRACE && peekToken()->type != END_OF_TOKENS) {
            addChild(node, parseStmt());
            if(hasError != 1) {
                advanceToken();
            } else {
                hasError = 0;
            }
        } 

         if(errorCheck(RIGHT_BRACE, "Expected '}' to close 'queenbee' block") == false) {
        advanceToken();
        return node;
            } else {
                errorRecovery();
                hasError = 1;
                addChild(node, createNode("Error"));
            }
    } else {
        errorRecovery();
        hasError = 1;
        addChild(node, createNode("Error"));
    }

   
    return node;
}

Node *parseStmt() {
    Node *node = createNode("<stmt>");
    if (peekToken()->type == GATHER_TOKEN) {
        return parseInputStmt();
    } else if (peekToken()->type == VAR_IDENT) {
        return parseAssignStmt();
    } else if (peekToken()->type == BUZZOUT_TOKEN) {
        return parseOutputStmt();
    } else if (peekToken()->type == IF_TOKEN || peekToken()->type == SWITCH_TOKEN) {
        return parseCondStmt();
    } else if (peekToken()->type == WHILE_TOKEN || peekToken()->type == FOR_TOKEN || peekToken()->type == DO_TOKEN) {
       // return parseIterStmt();
    } else if (peekToken()->type == INT_TOKEN || peekToken()->type == FLOAT_TOKEN || peekToken()->type == CHAR_TOKEN
             || peekToken()->type == CHAIN_TOKEN  || peekToken()->type == BOOL_TOKEN) {
        return parseDeclarationStmt();
    } else if (peekToken()->type == RETURN_TOKEN) {
      //  return parseReturnStmt(); 
    } else {
        errorMessage("Invalid statement");
        errorRecovery();
        hasError = 1;
        addChild(node, createNode("Error"));
    }
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
                addChild(node, createNode("Error"));
            }
        } else {
            errorMessage("Expected variable identifier after 'gather'");
            errorRecovery();
            hasError = 1;
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
        //addChild(node, parseSwitchStmt());
    }
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
                            Node *stmt = createNode("<stmt>");
                            addChild(node, stmt);
                            addChild(stmt, parseStmt());
                            advanceToken();
                        }
                        if(peekToken()->type == RIGHT_BRACE) {
                            advanceToken();
                            if(peekToken()->type == ELSEIF_TOKEN) {
                                while (peekToken()->type == ELSEIF_TOKEN) {
                                    addChild(node, parsElseIfStmt());
                                }
                            } else if(peekToken()->type == ELSE_TOKEN) {
                                    addChild(node, parseElseStmt());
                            } else {
                                currentToken--;
                                return node;
                            }
                        }
                    } else {
                        errorMessage("Expected a '{' after condition");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;       
                    }
                } else {
                    errorMessage("Expected a ')' after expression");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                }
            } else {
                errorMessage("Expected an expression inside 'if' condition");
                addChild(node, createNode("Error"));
                errorRecovery();
                hasError = 1;
            }
        } else {
            errorMessage("Expected a '(' after 'if'");
            errorRecovery();
            hasError = 1;
            addChild(node, createNode("Error"));
        }
    }
    return node;
}

Node *parsElseIfStmt() {
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
                                advanceToken();
                            }
                            if(peekToken()->type == RIGHT_BRACE) {
                                return node;
                            } else {
                                errorMessage("Expected a '}' to close 'elseif' block");
                                addChild(node, createNode("Error"));
                                errorRecovery();
                                hasError = 1;  
                            }
                        } else {
                            errorMessage("Expected a '{' after condition");
                            addChild(node, createNode("Error"));
                            errorRecovery();
                            hasError = 1;       
                        }
                    } else {
                        errorMessage("Expected a ')' after expression");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;
                    }
                } else {
                    errorMessage("Expected an expression inside 'elseif' condition");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                }
        } else {
            errorMessage("Expected a '(' after 'elseif'");
            errorRecovery();
            hasError = 1;
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
                advanceToken();
            }
            if(peekToken()->type == RIGHT_BRACE) {

                return node;
            } else {
                errorMessage("Expected a '}' to close 'elseif' block");
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

/*Node *parseSwitchStmt() {
    Node *node = createNode("<switch_stmt>");

    if(peekToken()->type == SWITCH_TOKEN) {
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
                        if(peekToken()->type == CASE_TOKEN) {
                            addChild(node, parseCaseClause());
                        } else if(peekToken()->type == DEFAULT_TOKEN) {
                            addChild(node, createNode(peekToken()->value()));
                            advanceToken();
                            if(peekToken()->type == COLON) {
                                addChild(node, createNode(peekToken()->value()));
                                advanceToken();
                            }
                        } else {
                            errorMessage("Expected 'case' or 'default");
                            addChild(node, createNode("Error"));
                            errorRecovery();
                            hasError = 1;
                        }
                    } else {
                        errorMessage("Expected a '{' after condition");
                        addChild(node, createNode("Error"));
                        errorRecovery();
                        hasError = 1;       
                    }
                } else {
                    errorMessage("Expected a ')' after expression");
                    addChild(node, createNode("Error"));
                    errorRecovery();
                    hasError = 1;
                }
            }
        } else {
                errorMessage("Expected '(' after 'switch");
                hasError = 1;
                errorRecovery();
                addChild(node, createNode("Error"));
        }
    }
}

Node *parseCaseClause() {

}

*/

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
                        errorRecovery();
                        addChild(node, createNode("Error"));
                    }
                } else if(peekToken()->type == DBL_QUOTE) {
                    string = strdup("\"\"");
                } else {
                    errorMessage("Missing closing quote");
                    hasError = 1;
                    errorRecovery();
                    addChild(node, createNode("Error"));
                }

                advanceToken();
                if (string) {
                    addChild(node, createNode(string)); // Add the string or empty string
                    free(string);
                }
        
                while (peekToken()->type == COMMA) {
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
                        return node;
                    } else {
                        hasError = 1;
                        errorRecovery();
                        addChild(node, createNode("Error"));
                    }
                } else {
                    hasError = 1;
                    errorRecovery();
                    addChild(node, createNode("Error"));
                }
            } else {
                errorMessage("Expected a string after '('");
                hasError = 1;
                errorRecovery();
                addChild(node, createNode("Error"));
            }
        } else {
            errorMessage("Expected '(' after 'buzzout'");
            hasError = 1;
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
        addChild(node, createNode("Error"));
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
    printf("Line %d   Error: %s\n", peekToken()->line, message);
    if (outputFile) {
        fprintf(outputFile, "Line %d   Error: %s\n", peekToken()->line, message);
    }
}