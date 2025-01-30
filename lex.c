/* 
   To do:
   Unterminated comments and strings (unsure if lexer or parser handles the error)
*/

#include "lex.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

unsigned int line = 1;

int tokens_index = 0;
int lexeme_index = 0;
int char_class;

#define COMMENT_CLASS 0
#define LETTER 1
#define DIGIT 2
#define OTHER 3

Token *lex(FILE *file) {
	int type;
	
	// stores all tokens
	int number_of_tokens = 12; // placeholder value for number of tokens
  	int tokens_size = 0;
    Token *tokens = malloc(sizeof(Token) * number_of_tokens); // array of all tokens in file
  	if (!tokens) {
        perror("Failed to allocate memory for tokens");
        exit(EXIT_FAILURE);
    }

	// finds length of file
	fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

	char *lexeme = malloc(sizeof(char) * (length + 1));
	if (!lexeme) {
        perror("Failed to allocate memory for lexeme");
        free(tokens);
        exit(EXIT_FAILURE);
    }
    
    char ch;
	
    while((ch = getNonBlank(file)) != EOF) {
    	lexeme[lexeme_index++] = ch; // builds lexeme by character
        
    	Token *token = malloc(sizeof(Token)); // allocates memory for current token
	    if (!token) {
            perror("Failed to allocate memory for token");
            free(tokens);
            free(lexeme);
            exit(EXIT_FAILURE);
        }
        tokens_size++;

	    if (tokens_size >= number_of_tokens) {
            reallocMemory(&tokens, &number_of_tokens);
        }

        int state = 0;
		// limit function calls by grouping tokens based on composition
		switch(char_class) {
			case COMMENT_CLASS:
                lexeme[lexeme_index++] = getc(file);
                storeToken(token, tokens, lexeme, COMMENT_BEGIN);

                lexeme_index = 0;
                Token *token1 = malloc(sizeof(Token));;

                while(state != 2) {
                    ch = getNextChar(file);

                    if(ch == EOF) {
                        storeToken(token1, tokens, lexeme, COMMENT);
                        tokens_size++;
                        if (tokens_size >= number_of_tokens) {
                            reallocMemory(&tokens, &number_of_tokens);
                        }
                        break; // Exit early for EOF
                    }

                    switch (state) {
                        case 0:
                            if (ch == ':') {
                                state = 1;
                                
                            } else {
                                lexeme[lexeme_index++] = ch;
                            }
                            break;
                        case 1: 
                            if (ch == '>') {
                                state = 2; // accept the input
                            } else {
                                lexeme[lexeme_index++] = ':';
                                lexeme[lexeme_index++] = ch;
                                state = 0; // check for ':' input again
                            }
                            break;
                    }
                } // end of while(state != 2)

                if(lexeme_index != 0 && ch != EOF){
                    // Store the non-blank block comment content
                    storeToken(token, tokens, lexeme, COMMENT);
                    tokens_size++;
                    if (tokens_size >= number_of_tokens) {
                        reallocMemory(&tokens, &number_of_tokens);
                    }
                }

                if(state == 2) {
                    lexeme_index = 2;
                    Token *token2 = malloc(sizeof(Token));;
                    strcpy(lexeme, ":>");
                    storeToken(token2, tokens, lexeme, COMMENT_END);
                    tokens_size++;
                    if (tokens_size >= number_of_tokens) {
                        reallocMemory(&tokens, &number_of_tokens);
                    }
                }
				break;

			case LETTER: // tokens containing only letters
                if(isKeywordReservedword(token, tokens, lexeme, ch, &type, file)) {
                    storeToken(token, tokens, lexeme, type);
				} else {
		    		storeToken(token, tokens, lexeme, INVALID);
				}
		    	break;
		    	
			case DIGIT: // numeric literals
				if(isNumLiteral(lexeme, ch, &type, file)) {
					storeToken(token, tokens, lexeme, type);
				} else {
					storeToken(token, tokens, lexeme, INVALID);
				}
				break;
				
			case OTHER: // tokens that have non-alpanumeric characters
				if(isIdentifier(lexeme, ch, &type, file)) {
          		  	storeToken(token, tokens, lexeme, type);
		        } else if(isDelimiter(ch, &type)) {
					storeToken(token, tokens, lexeme, type);

                    if(type == DBL_QUOTE) {
                        lexeme_index = 0;
                        ch = getNextChar(file);
                        
                        while(ch != '"' && ch != '\n' && ch != EOF) {
                            lexeme[lexeme_index++] = ch; 
                            ch = getNextChar(file);
                        }

                        Token *token = malloc(sizeof(Token)); // allocates memory for current token
                        storeToken(token, tokens, lexeme, STRING);
                        tokens_size++;
                        if (tokens_size >= number_of_tokens) {
                            reallocMemory(&tokens, &number_of_tokens);
                        }
                        lexeme_index = 0;

                        if(ch == '"') {
                            Token *token = malloc(sizeof(Token));
                            lexeme[lexeme_index++] = ch;
                            storeToken(token, tokens, lexeme, DBL_QUOTE);
                            lexeme_index = 0;
                            tokens_size++;
                            if (tokens_size >= number_of_tokens) {
                                reallocMemory(&tokens, &number_of_tokens);
                            }
                        } else {
                            ungetc(ch, file);
                        }
                    } else if(type == SNGL_QUOTE){                                    
                        lexeme_index = 0;
                        ch = getNextChar(file);

                        if(ch != EOF && ch != '\n' && ch != '\''){
                            lexeme[lexeme_index++] = ch;
                            Token *token = malloc(sizeof(Token));
                            storeToken(token, tokens, lexeme, CHARACTER);
                            tokens_size++;
                            if (tokens_size >= number_of_tokens) {
                                reallocMemory(&tokens, &number_of_tokens);
                            }
                            lexeme_index = 0;
                            ch = getNextChar(file);
                        }

                        if(ch == '\''){
                            Token *token = malloc(sizeof(Token));
                            lexeme[lexeme_index++] = ch;
                            storeToken(token, tokens, lexeme, SNGL_QUOTE);
                            tokens_size++;
                            if (tokens_size >= number_of_tokens) {
                                reallocMemory(&tokens, &number_of_tokens);
                            }
                        } else{
                            ungetc(ch, file);
                        }
                    }
				} else if(isOperator(lexeme, ch, &type, file)) {
            		storeToken(token, tokens, lexeme, type);
            	} else {
	        		storeToken(token, tokens, lexeme, INVALID);
				}
            	break;
            	
			default:
				strcpy("Unknown exception", lexeme);
	        	storeToken(token, tokens, lexeme, INVALID);
		} // end of switch(char_class)
		lexeme_index = 0;	
        if(ch == EOF){
            break;
        }
    } // end of while(ch != EOF)

    tokens[tokens_index].value = NULL; // appends null terminator to indicate the end of tokens
    tokens[tokens_index].type = END_OF_TOKENS;
    tokens[tokens_index].line = line;
    free(lexeme);
    return tokens;
} //end of Token *lex

int isNumLiteral(char *lexeme, char ch, int *type, FILE *file) {
	int has_decimal = 0;

    ch = getNextChar(file);
    while(isdigit(ch) || ch == '.') {
    	if(ch == '.' && has_decimal == 1) {
    		while(isdigit(ch) || ch == '.') { // incorrect float format
				lexeme[lexeme_index++] = ch;  // store any subsequent numbers and decimal points then return INVALID
    			ch = getNextChar(file);
			}
			return 0;
		} else if(ch == '.' && has_decimal == 0) {
			has_decimal = 1;
		}
    	
    	lexeme[lexeme_index++] = ch;
    	ch = getNextChar(file);
	}
            
	ungetc(ch, file);  // Put back the non-numeric character
    lexeme[lexeme_index] = '\0';
        
    if(has_decimal) {
    	*type = FLOAT;
	} else {
		*type = INTEGER;
	}
	
	return 1;
}

int isIdentifier(char *lexeme, char ch, int *type, FILE *file) {
	int state = 0;
	
    switch (state) {
    	case 0: // start state
            if (ch == '#' || ch == '~') {
            	state = 1;
            	ch = getNextChar(file);
            } else {
        		return 0; // not a variable
            }
        case 1: 
            if (isalpha(ch)) {
                state = 2; // first char is valid, move to state 2
                lexeme[lexeme_index++] = ch;
                ch = getNextChar(file);
            } else {
            	while(!(isspace(ch) || ch == '\n' || ch == '\t')) { // take entire invalid string
            		lexeme[lexeme_index++] = ch;
                	ch = getNextChar(file);
            	}
            	ungetc(ch, file);
            	*type = INVALID;
                return 0; // invalid variable
            }
        case 2: // checks if next character is valid
			while(isalpha(ch) || isdigit(ch)) {
	        	lexeme[lexeme_index++] = ch;
	            ch = getNextChar(file);
            }
            
            ungetc(ch, file);
            if(lexeme[0] == '#') {
            	*type = VAR_IDENT;
			} else {
				*type = FUNC_IDENT;
			}
           	return 1;
        default:
            return 0; // Invalid variable
    }
}

int isDelimiter(char ch, int *type) {
	switch (ch) {
        case ';':
    		*type = SEMICOLON;
    		return 1;
        case ',':
    		*type = COMMA;
    		return 1;
        case '(':
    		*type = LEFT_PAREN;
    		return 1;
        case ')':
    		*type = RIGHT_PAREN;
    		return 1;
        case '[':
    		*type = LEFT_BRACKET;
    		return 1;
        case ']':
    		*type = RIGHT_BRACKET;
    		return 1;
        case '{':
    		*type = LEFT_BRACE;
    		return 1;
        case '}':
    		*type = RIGHT_BRACE;
    		return 1;
        case '"':
    		*type = DBL_QUOTE;
    		return 1;
        case '\'':
            *type = SNGL_QUOTE;
    		return 1;
        case ':':
            *type = COLON;
            return 1;
        default:
            return 0; // not a delimiter
    }
}

int isOperator(char *lexeme, char ch, int *type, FILE *file) {
	switch(ch) {
    	case '+':	
    		*type = ADDITION;
            ch = getc(file);
            if(ch == '+') {
                *type = INCREMENT;
                lexeme[lexeme_index++] = ch;
                return 1;
            } else {
				ungetc(ch, file);
	    	}
    		return 1;
    	case '-':
    		*type = SUBTRACTION;
            ch = getc(file);
            if(ch == '-') {
                *type = DECREMENT;
                lexeme[lexeme_index++] = ch;
                return 1;
            } else {
				ungetc(ch, file);
	    	}
    		return 1;
    	case '*':
    		*type = MULTIPLICATION;
    		return 1;
    	case '/':
            ch = getc(file);
            if(ch == '/') {
                *type = INT_DIVISION;
                lexeme[lexeme_index++] = ch;
                return 1;
            } else {
                *type = DIVISION;
				ungetc(ch, file);
                return 1;
	    	}
    	case '%':
    		*type = MODULO;
    		return 1;
    	case '^':
    		*type = EXPONENT;
    		return 1;
		case '>':
    		ch = getNextChar(file);
    		if(ch == '=') {
    			lexeme[lexeme_index++] = ch;
    			*type = GREATER_EQUAL;
    			return 1;
			} else {
				ungetc(ch, file);
	    		*type = GREATER_THAN;
	    		return 1;	
	    	}
    	case '<':
    		ch = getNextChar(file);
    		if(ch == '=') {
    			lexeme[lexeme_index++] = ch;
    			*type = LESS_EQUAL;
    			return 1;
			} else {
				ungetc(ch, file);
	    		*type = LESS_THAN;
	    		return 1;	
	    	}
    	case '=':
    		ch = getNextChar(file);
    		if(ch == '=') {
    			lexeme[lexeme_index++] = ch;
    			*type = IS_EQUAL_TO;
    			return 1;
			} else {
				ungetc(ch, file);
	    		*type = ASSIGNMENT_OP;
	    		return 1;	
	    	}
	   	case '&':
    		ch = getNextChar(file);
    		if(ch == '&') {
    			lexeme[lexeme_index++] = ch;
    			*type = AND;
    			return 1;
			} else {
				ungetc(ch, file);
				return 0;
	    	}
	    case '|':
    		ch = getNextChar(file);
    		if(ch == '|') {
    			lexeme[lexeme_index++] = ch;
    			*type = OR;
    			return 1;
			} else {
				ungetc(ch, file);
				return 0;
	    	}
	   	case '!':
    		ch = getNextChar(file);
    		if(ch == '=') {
    			lexeme[lexeme_index++] = ch;
    			*type = NOT_EQUAL;
    			return 1;;
			} else {
				ungetc(ch, file);
	    		*type = NOT;
	    		return 1;	
	    	}
    	default:
    		return 0; // not an operator
	}
}

int isKeywordReservedword(Token *token, Token *tokens, char *lexeme, char ch, int *type, FILE *file) {
    int notKeyReserveWord = 0;
    
    
    switch(ch) {
        case 'b':
            ch = getc(file);
            if(ch == 'e') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'e') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'g') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 'i') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(ch == 'n') {
                                lexeme[lexeme_index++] = ch;
                                ch = getc(file);
                                if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) { //beegin
                                    lexeme[lexeme_index] = '\0';
                                    ungetc(ch, file);
                                    *type = BEEGIN_TOKEN;
                                    return 1;
                                } else {
                                    notKeyReserveWord = 1;
                                }
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else if(ch == 'o') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(ch == 'n') {
                                lexeme[lexeme_index++] = ch;
                                ch = getc(file);
                                if(ch == 'e') {
                                    lexeme[lexeme_index++] = ch;
                                    ch = getc(file);
                                    if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                        lexeme[lexeme_index] = '\0';
                                        ungetc(ch, file);
                                        *type = BEEGONE_TOKEN;
                                        return 1;
                                    } else {
                                        notKeyReserveWord = 1;
                                    }
                                } else {
                                    notKeyReserveWord = 1; 
                                }
                            } else {
                                notKeyReserveWord = 1;
                            }  
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else if(ch == 'o') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'o') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'l') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                            lexeme[lexeme_index] = '\0';
                            ungetc(ch, file);
                            *type = BOOL_TOKEN;
                            return 1;
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else if(ch == 'u') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'z') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'z') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                            lexeme[lexeme_index] = '\0';
                            ungetc(ch, file);
                            *type = BUZZ_TOKEN;
                            return 1;
                        } else if(ch == 'o') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(ch == 'u') {
                                lexeme[lexeme_index++] = ch;
                                ch = getc(file);
                                if(ch == 't') {
                                    lexeme[lexeme_index++] = ch;
                                    ch = getc(file);
                                    if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                        lexeme[lexeme_index] = '\0';
                                        ungetc(ch, file);
                                        *type = BUZZOUT_TOKEN;
                                        return 1;
                                    } else {
                                        notKeyReserveWord = 1;
                                    }
                                } else {
                                    notKeyReserveWord = 1;
                                }
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else {
                notKeyReserveWord = 1;
            }
            break;
        case 'c':
            ch = getc(file);
            if(ch == 'a') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 's') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'e') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                            lexeme[lexeme_index] = '\0';
                            ungetc(ch, file);
                            *type = CASE_TOKEN;
                            return 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                        notKeyReserveWord = 1;
                }
            } else if(ch == 'h') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'a') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'i') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 'n') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                lexeme[lexeme_index] = '\0';
                                ungetc(ch, file);
                                *type = CHAIN_TOKEN;
                                return 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else if(ch == 'r') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                            lexeme[lexeme_index] = '\0';
                            ungetc(ch, file);
                            *type = CHAR_TOKEN;
                            return 1;
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else {
                notKeyReserveWord = 1;
            }
            break;
        case 'd':
            ch = getc(file);
            if(ch == 'e') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'f') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'a') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 'u') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(ch == 'l') {
                                lexeme[lexeme_index++] = ch;
                                ch = getc(file);
                                if(ch == 't') {
                                    lexeme[lexeme_index++] = ch;
                                    ch = getc(file);
                                    if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                        lexeme[lexeme_index] = '\0';
                                        ungetc(ch, file);
                                        *type = DEFAULT_TOKEN;
                                        return 1;
                                    }
                                } else {
                                    notKeyReserveWord = 1;
                                }
                            }  else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else if(ch == 'o') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                    lexeme[lexeme_index] = '\0';
                    ungetc(ch, file);
                    *type = DO_TOKEN;
                    return 1;
                }
            } else {
                notKeyReserveWord = 1;
            }
            break;
        case 'e':
            ch = getc(file);
            if(ch == 'l') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 's') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'e') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                            lexeme[lexeme_index] = '\0';
                            ungetc(ch, file);
                            *type = ELSE_TOKEN;
                            return 1;
                        } else if (ch == 'i') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(ch == 'f') {
                                lexeme[lexeme_index++] = ch;
                                ch = getc(file);
                                if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                    lexeme[lexeme_index] = '\0';
                                    ungetc(ch, file);
                                    *type = ELSEIF_TOKEN;
                                    return 1;
                                } else {
                                    notKeyReserveWord = 1;
                                }
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else {
                notKeyReserveWord = 1;
            }
            break;
        case 'f':
            ch = getc(file);
            if(ch == 'a') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'l') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 's') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 'e') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                lexeme[lexeme_index] = '\0';
                                ungetc(ch, file);
                                *type = FALSE_TOKEN;
                                return 1;
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else if(ch == 'l') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'o') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'a') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 't') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                lexeme[lexeme_index] = '\0';
                                ungetc(ch, file);
                                *type = FLOAT_TOKEN;
                                return 1;
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else if(ch == 'o') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'r') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                        lexeme[lexeme_index] = '\0';
                        ungetc(ch, file);
                        *type = FOR_TOKEN;
                        return 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else {
                    notKeyReserveWord = 1;
            }
            break;
        case 'g':
            ch = getc(file);
            if(ch == 'a') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 't') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'h') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 'e') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(ch == 'r') {
                                lexeme[lexeme_index++] = ch;
                                ch = getc(file);
                                    if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                        lexeme[lexeme_index] = '\0';
                                        ungetc(ch, file);
                                        *type = GATHER_TOKEN;
                                        return 1;
                                    } else {
                                        notKeyReserveWord = 1;
                                    }
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } 
            break;
        case 'i':
            ch = getc(file);
            if(ch == 'f') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                    lexeme[lexeme_index] = '\0';
                    ungetc(ch, file);
                    *type = IF_TOKEN;
                    return 1;
                } else {
                    notKeyReserveWord = 1;
                }
            } else if(ch == 'n') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 't') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                        lexeme[lexeme_index] = '\0';
                        ungetc(ch, file);
                        *type = INT_TOKEN;
                        return 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            }
            break;
        case 'h':
            ch = getc(file);
            if(ch == 'i') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'v') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'e') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                            lexeme[lexeme_index] = '\0';
                            ungetc(ch, file);
                            *type = HIVE_TOKEN;
                            return 1;
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else if(ch == 'o') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'v') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'e') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 'r') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                lexeme[lexeme_index] = '\0';
                                ungetc(ch, file);
                                *type = HOVER_TOKEN;
                                return 1;
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } 
            break;
        case 'q':
            ch = getc(file);
            if(ch == 'u') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 'e') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'e') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 'n') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(ch == 'b') {
                                lexeme[lexeme_index++] = ch;
                                ch = getc(file);
                                if(ch == 'e') {
                                    lexeme[lexeme_index++] = ch;
                                    ch = getc(file);
                                        if(ch == 'e') {
                                            lexeme[lexeme_index++] = ch;
                                            ch = getc(file);
                                            if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                                lexeme[lexeme_index] = '\0';
                                                ungetc(ch, file);
                                                *type = QUEENBEE_TOKEN;
                                                return 1;
                                            }
                                        } else {
                                            notKeyReserveWord = 1;
                                        }
                                } else {
                                    notKeyReserveWord = 1;
                                }
                            }  else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            }
            break;
        case 'r': // return
            ch = getc(file);
            if(ch == 'e') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if(ch == 't') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if(ch == 'u') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(ch == 'r') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(ch == 'n') {
                                lexeme[lexeme_index++] = ch;
                                ch = getc(file);
                                if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                    lexeme[lexeme_index] = '\0';
                                    ungetc(ch, file);
                                    *type = RETURN_TOKEN;
                                    return 1;
                                } else if(ch == 'v') {
                                    lexeme[lexeme_index++] = ch;
                                    ch = getc(file);
                                    if(ch == 'a') {
                                        lexeme[lexeme_index++] = ch;
                                        ch = getc(file);
                                        if(ch == 'l') {
                                            lexeme[lexeme_index++] = ch;
                                            ch = getc(file);
                                            if(ch == 'u') {
                                                lexeme[lexeme_index++] = ch;
                                                ch = getc(file);
                                                    if(ch == 'e') {
                                                        lexeme[lexeme_index++] = ch;
                                                        ch = getc(file);
                                                        if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                                            strcpy(lexeme, "return"); // store "return" as token first
                                                            lexeme_index = 6;
                                                            *type = RETURN_TOKEN;
                                                            storeToken(token, tokens, lexeme, *type);
                                                            
                                                            strcpy(lexeme, "value"); // store "value" noise word
                                                            lexeme_index = 5;
                                                            token = malloc(sizeof(Token));;
                                                            *type = NOISE_WORD;
                                                            return 1;
                                                        }
                                                    } else {
                                                        notKeyReserveWord = 1;
                                                    }
                                            } else {
                                                notKeyReserveWord = 1;
                                            }
                                        }  else {
                                            notKeyReserveWord = 1;
                                        }
                                    } else {
                                        notKeyReserveWord = 1;
                                    }
                                } else {
                                    notKeyReserveWord = 1;
                                }
                            }  else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            }
            break;

        case 'w':       // while
            ch = getc(file);
            if (ch == 'h') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if (ch == 'i') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if (ch == 'l') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if (ch == 'e') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                lexeme[lexeme_index] = '\0';
                                ungetc(ch, file);
                                *type = WHILE_TOKEN;
                                return 1;
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else {
                notKeyReserveWord = 1;
            }
            break;
        
        case 's':
            ch = getc(file);
            if (ch == 't') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if (ch == 'i') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if (ch == 'n') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if (ch == 'g') {
                            if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                lexeme[lexeme_index] = '\0';
                                ungetc(ch, file);
                                *type = STING_TOKEN;
                                return 1;
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else if (ch == 'w') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if (ch == 'i') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if (ch == 't') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if (ch == 'c') {
                            lexeme[lexeme_index++] = ch;
                            ch = getc(file);
                            if (ch == 'h') {
                                if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                                    lexeme[lexeme_index] = '\0';
                                    ungetc(ch, file);
                                    *type = SWITCH_TOKEN;
                                    return 1;
                                } else {
                                    notKeyReserveWord = 1;
                                }
                            } else {
                                notKeyReserveWord = 1;
                            }
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else {
                notKeyReserveWord = 1;
            }
            break;
        case 't':
            ch = getc(file);
            if (ch == 'r') {
                lexeme[lexeme_index++] = ch;
                ch = getc(file);
                if (ch == 'u') {
                    lexeme[lexeme_index++] = ch;
                    ch = getc(file);
                    if (ch == 'e') {
                        lexeme[lexeme_index++] = ch;
                        ch = getc(file);
                        if(isspace(ch) || ch == '\n' || ispunct(ch) || ch == EOF) {
                            lexeme[lexeme_index] = '\0';
                            ungetc(ch, file);
                            *type = TRUE_TOKEN;
                            return 1;
                        } else {
                            notKeyReserveWord = 1;
                        }
                    } else {
                        notKeyReserveWord = 1;
                    }
                } else {
                    notKeyReserveWord = 1;
                }
            } else {
                notKeyReserveWord = 1;
            }
            break;
        default: 
            ch = getc(file);
            while(isalpha(ch) && ch != '\n' && ch != EOF) {
                    lexeme[lexeme_index++] = ch;
                    ch = getNextChar(file);
            } 
                    
            if(isdigit(ch)) {
                while(!(isspace(ch)) && ch != '\n' && ch != EOF) {
                    lexeme[lexeme_index++] = ch;
                    ch = getNextChar(file);
                }
            }
                    
            lexeme[lexeme_index] = '\0';
            *type = INVALID;
            ungetc(ch, file);
            return 0;
            
    }

    if(notKeyReserveWord == 1) { // not a keyword or reserved word, take the entire invalid string
            while(isalpha(ch) && ch != '\n' && ch != EOF) {
                    lexeme[lexeme_index++] = ch;
                    ch = getNextChar(file);
            } 
                    
            if(isdigit(ch)) { // takes lexemes beginning with letters but containing numbers
                while(!(isspace(ch)) && ch != '\n' && ch != EOF) {
                    lexeme[lexeme_index++] = ch;
                    ch = getNextChar(file);
                }
            }
                    
            lexeme[lexeme_index] = '\0';
            *type = INVALID;
            ungetc(ch, file);
            return 0;
    } else {
        return 1;
    }
}

// return any character including spaces or newlines
char getNextChar(FILE *file) {
	char ch = fgetc(file), temp;
	
	// seperate characters by class
	if(ch == '<') {
		temp = getc(file);
        if(temp == '|') {
            char_class = COMMENT_CLASS;
            ungetc(temp, file);
        } else {
            char_class = OTHER;
            ungetc(temp, file);
        }
	} else if(isalpha(ch)) {
		char_class = LETTER;
	} else if(isdigit(ch)) {
		char_class = DIGIT;
	} else {
		char_class = OTHER;
	} 

	return ch;
}

// skip whitespaces and newline
char getNonBlank(FILE *file) {
	char ch = getNextChar(file);
	
	while(isspace(ch) || ch == '\t' || ch == '\n') {
		if(ch == '\n') {
    		line++;
		}
		ch = getNextChar(file);
	}
	
	return ch;
}

void storeToken(Token *token, Token *tokens, char *lexeme, int type) {
    lexeme[lexeme_index] = '\0';
    token->value = malloc(strlen(lexeme) + 1);
    strcpy(token->value, lexeme);
    token->line = line;
    token->type = type;
    
    tokens[tokens_index] = *token;
    tokens_index++;
}

void reallocMemory(Token **tokens, int *number_of_tokens) {
    *number_of_tokens *= 2;
    Token *new_tokens = realloc(*tokens, sizeof(Token) * *number_of_tokens);
    if (!new_tokens) {
        perror("Failed to reallocate memory for tokens");
        free(*tokens);
        exit(EXIT_FAILURE);
    }
    *tokens = new_tokens; // Update the caller's pointer
}