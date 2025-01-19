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
				ch = getNextChar(file);
				while(isalpha(ch) && ch != '\n' && ch != EOF) {
					lexeme[lexeme_index++] = ch;
					ch = getNextChar(file);
				} 
				
				if(isdigit(ch)) { // takes lexemes beginning with letters but containing numbers
					while(!(isspace(ch)) && ch != '\n' && ch != EOF) {
						lexeme[lexeme_index++] = ch;
						ch = getNextChar(file);
					}
					storeToken(token, tokens, lexeme, INVALID);
					break;
				}
				
				lexeme[lexeme_index] = '\0';
				ungetc(ch, file);
				
				if(isKeyword(token, tokens, lexeme, ch, &type)) {
		            storeToken(token, tokens, lexeme, type);
		    	} else if(isReservedWord(lexeme, ch, &type)) {
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

int isKeyword(Token *token, Token *tokens, char *lexeme, char ch, int *type) {
	int i = 0;  // Index for lexeme
    ch = lexeme[i];  // Start by checking the first character

    switch(ch) {
        case 'b':  // Start with 'b' for the keyword 'beegin'
            if (lexeme[i + 1] == 'e' && 
                lexeme[i + 2] == 'e' && 
                lexeme[i + 3] == 'g' && 
                lexeme[i + 4] == 'i' && 
                lexeme[i + 5] == 'n' &&
                lexeme[i + 6] == '\0') {
                *type = BEEGIN_TOKEN;
                return 1;  // Matched 'beegin'
            }
            else if (lexeme[i + 1] == 'e' && 
                lexeme[i + 2] == 'e' && 
                lexeme[i + 3] == 'g' && 
                lexeme[i + 4] == 'o' &&
                lexeme[i + 5] == 'n' &&
                lexeme[i + 6] == 'e'&&
                lexeme[i + 7] == '\0'){
                *type = BEEGONE_TOKEN;
                return 1; //matched 'beegone'
            }
            else if (lexeme[i + 1] == 'u' && 
                lexeme[i + 2] == 'z' && 
                lexeme[i + 3] == 'z' && 
                lexeme[i + 4] == 'o' &&
                lexeme[i + 5] == 'u' &&
                lexeme[i + 6] == 't'&&
                lexeme[i + 7] == '\0') {
                *type = BUZZOUT_TOKEN;
            	return 1;  // Matched 'buzzout'
            }
            else if (lexeme[i + 1] == 'u' && 
                lexeme[i + 2] == 'z' && 
                lexeme[i + 3] == 'z'&&
                lexeme[i + 4] == '\0') {
                *type = BUZZ_TOKEN;
                return 1;  // Matched 'buzz'
            }
            break;

        case 'c':  // Start with 'c' for the keyword 'case'
            if (lexeme[i + 1] == 'a' && 
                lexeme[i + 2] == 's' && 
                lexeme[i + 3] == 'e'&&
                lexeme[i + 4] == '\0') {
                *type = CASE_TOKEN;
                return 1;  // Matched 'case'
            }
            break;

        case 'd':  // Start with 'd' for the keyword 'do' or 'downto'
            if (lexeme[i + 1] == 'o'&&
                lexeme[i + 2] == '\0') {
                *type = DO_TOKEN;
                return 1;  // Matched 'do'
            } 
            else if(lexeme[i + 1] == 'e' && 
                lexeme[i + 2] == 'f' && 
                lexeme[i + 3] == 'a' &&
                lexeme[i + 4] == 'u' && 
                lexeme[i + 5] == 'l'&&
                lexeme[i + 6] == 't' &&
                lexeme[i + 7] == '\0') {
                *type = DEFAULT_TOKEN;
                return 1;
            }
            break;

        case 'e':  // Start with 'e' for the keyword 'else' or 'elseif'
            if (lexeme[i + 1] == 'l' && 
                lexeme[i + 2] == 's' && 
                lexeme[i + 3] == 'e' &&
                lexeme[i + 4] == 'i' && 
                lexeme[i + 5] == 'f'&&
                lexeme[i + 6] == '\0') {
                *type = ELSEIF_TOKEN;
                return 1;  // Matched 'elseif'
            } 
            else if (lexeme[i + 1] == 'l' && 
                lexeme[i + 2] == 's' && 
                lexeme[i + 3] == 'e'&&
                lexeme[i + 4] == '\0') {
                *type = ELSE_TOKEN;
                return 1;  // Matched 'else'
            }
            break;

        case 'f':  // Start with 'f' for the keyword 'for'
            if (lexeme[i + 1] == 'o' && 
                lexeme[i + 2] == 'r'&&
                lexeme[i + 3] == '\0') {
                *type = FOR_TOKEN;
                return 1;  // Matched 'for'
            }
            break;

        case 'g':  // Start with 'g' for the keyword 'gather'
            if (lexeme[i + 1] == 'a' && 
                lexeme[i + 2] == 't' && 
                lexeme[i + 3] == 'h' && 
                lexeme[i + 4] == 'e' && 
                lexeme[i + 5] == 'r'&&
                lexeme[i + 6] == '\0') {
                *type = GATHER_TOKEN;
                return 1;  // Matched 'gather'
            }
            break;

        case 'h':  // Start with 'h' for the keyword 'hive'
            if (lexeme[i + 1] == 'i' && 
                lexeme[i + 2] == 'v' && 
                lexeme[i + 3] == 'e'&&
                lexeme[i + 4] == '\0') {
                *type = HIVE_TOKEN;
                return 1;  // Matched 'hive'
           }
            else if (lexeme[i + 1] == 'o' &&
                lexeme[i + 2] == 'v' &&
                lexeme[i + 3] == 'e' &&
                lexeme[i + 4] == 'r' &&
                lexeme[i + 5] == '\0') {
                *type = HOVER_TOKEN;
                return 1; //matched hover
            }
            break;

        case 'i':  // Start with 'i' for the keyword 'if' or 'is'
            if (lexeme[i + 1] == 'f' &&
                lexeme[i + 2] == '\0') {
                *type = IF_TOKEN;
                return 1;  // Matched 'if'
            }
            break;

        case 'q':  // Start with 'q' for the keyword 'queenbee'
            if (lexeme[i + 1] == 'u' && 
                lexeme[i + 2] == 'e' && 
                lexeme[i + 3] == 'e' && 
                lexeme[i + 4] == 'n' && 
                lexeme[i + 5] == 'b' && 
                lexeme[i + 6] == 'e' && 
                lexeme[i + 7] == 'e' &&
                lexeme[i + 8] == '\0') {
                *type = QUEENBEE_TOKEN;
                return 1;  // Matched 'queenbee'
            }
            break;

        case 'r':  // Start with 'r' for the keyword 'return'
            if (lexeme[i + 1] == 'e' && 
                lexeme[i + 2] == 't' && 
                lexeme[i + 3] == 'u' && 
                lexeme[i + 4] == 'r' && 
                lexeme[i + 5] == 'n') {
                if(lexeme[i + 6] == '\0') {
                    *type = RETURN_TOKEN;
                    return 1;// Matched 'returns'
                } else if(lexeme[i + 6] == 'v' &&
                    lexeme[i + 7] == 'a' &&
                    lexeme[i + 8] == 'l' &&
                    lexeme[i + 9] == 'u' &&
                    lexeme[i + 10] == 'e' &&
                    lexeme[i + 11] == '\0'){

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
            }
            break;

        case 's':  // Start with 's' for the keyword 'size', 'sting', 'switch'
            if (lexeme[i + 1] == 't' && 
                lexeme[i + 2] == 'i' && 
                lexeme[i + 3] == 'n' && 
                lexeme[i + 4] == 'g' &&
                lexeme[i + 5] == '\0') {
                *type = STING_TOKEN;
                return 1;  // Matched 'sting'
            }
            else if (lexeme[i + 1] == 'w' && 
                lexeme[i + 2] == 'i' && 
                lexeme[i + 3] == 't' && 
                lexeme[i + 4] == 'c' && 
                lexeme[i + 5] == 'h' &&
                lexeme[i + 6] == '\0') {
                *type = SWITCH_TOKEN;
                return 1;  // Matched 'switch'
            }
            break;

        case 'w':  // Start with 'w' for the keyword 'while'
            if (lexeme[i + 1] == 'h' && 
                lexeme[i + 2] == 'i' && 
                lexeme[i + 3] == 'l' && 
                lexeme[i + 4] == 'e' &&
                lexeme[i + 5] == '\0') {
                *type = WHILE_TOKEN;
                return 1;  // Matched 'while'
            }
            break;
        default:
        	return 0;
    }
    return 0;
}

int isReservedWord(char *lexeme, char ch, int *type) {
	int i = 0;  // Index for lexeme
    ch = lexeme[i];  // Start by checking the first character

	switch(ch){
        case 'b':
            if (lexeme[i + 1] == 'o' &&
                lexeme[i + 2] == 'o' &&
                lexeme[i + 3] == 'l' &&
                lexeme[i + 4] == '\0') {
                *type = BOOL_TOKEN;
                return 1; //matched bool
            }
            break;

        case 'c':
            if (lexeme[i + 1] == 'h' &&
                lexeme[i + 2] == 'a' &&
                lexeme[i + 3] == 'r' &&
                lexeme[i + 4] == '\0') {
                *type = CHAR_TOKEN;
                return 1; //matched char
            }
            else if (lexeme[i + 1] == 'h' &&
                lexeme[i + 2] == 'a' &&
                lexeme[i + 3] == 'i' &&
                lexeme[i + 4] == 'n' &&
                lexeme[i + 5] == '\0' ) {
                *type = CHAIN_TOKEN;
                return 1; //matched chain
                }
            break;
            
        case 'i':
            if (lexeme[i + 1] == 'n' &&
                lexeme[i + 2] == 't' &&
                lexeme[i + 3] == '\0') {
                *type = INT_TOKEN;
                return 1; //matched int
            }
            break;

        case 'f':
            if (lexeme[i + 1] == 'l' &&
                lexeme[i + 2] == 'o' &&
                lexeme[i + 3] == 'a' &&
                lexeme[i + 4] == 't' &&
                lexeme[i + 5] == '\0') {
                *type = FLOAT_TOKEN;
                return 1; //matched float
            }
            else if (lexeme[i + 1] == 'a' &&
                lexeme[i + 2] == 'l' &&
                lexeme[i + 3] == 's' &&
                lexeme[i + 4] == 'e' &&
                lexeme[i + 5] == '\0') {
                *type = FALSE_TOKEN;
                return 1;
                }
            break;

        case 't':
            if(lexeme[i + 1] == 'r' &&
                lexeme[i + 2] == 'u' &&
                lexeme[i + 3] == 'e' &&
                lexeme[i + 4] == '\0') {
                *type = TRUE_TOKEN;
                return 1;
                }
               break;
    }
    return 0;  // Not a reserved word
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