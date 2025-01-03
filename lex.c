/* DIV is returned as INVALID as the operator is in letters
   and isOperator() is in the OTHER class. Code for classification
   is already functional though.
   
   Will fix it kung di na tinatamad (gusto ko kaso palitan nalang ng symbols yung INT_DIV)

   To do:
   Optimize code for STRING classification
   File extension validation
   Remove unused variables in functions
   Unterminated comments and strings (unsure if lexer or parser handles the error)
   DIV
*/

#include "lex.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

unsigned int line = 1;
unsigned int column = 0;

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
  	
	// finds length of file
	fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

	// stores current lexeme
	char *lexeme = malloc(sizeof(char) * (length + 1));
	char ch;
	
    while((ch = getNonBlank(file)) != EOF) {
    	lexeme[lexeme_index++] = ch; // builds lexeme by character
    	
    	Token *token = malloc(sizeof(Token)); // allocates memory for current token
	    tokens_size++;
	    
	    if (tokens_size >= number_of_tokens) {
            number_of_tokens *= 2; // double the capacity
            Token *new_tokens = realloc(tokens, sizeof(Token) * number_of_tokens);
            tokens = new_tokens;
        }
		
		// limit function calls by grouping tokens based on composition
		switch(char_class) {
			case COMMENT_CLASS:
                int state = 0;

                while(state != 2) {
                    ch = getNextChar(file);
                    switch (state) {
                        case 0:
                            if (ch == ':') {
                                lexeme[lexeme_index++] = ch;
                                state = 1;
                            } else {
                                lexeme[lexeme_index++] = ch;
                            }
                            break;
                        case 1: 
                            if (ch == '>') {
                                lexeme[lexeme_index++] = ch;
                                state = 2; // accept the input
                            } else {
                                state = 0; // check for ':' input again
                            }
                            break;
                    }
                } // end of while(state != 2)

                storeToken(token, tokens, lexeme, COMMENT);
				break;

			case LETTER: // tokens containing only letters
				ch = getNextChar(file);
				while(isalpha(ch) && ch != '\n') {
					lexeme[lexeme_index++] = ch;
					ch = getNextChar(file);
				} 
				
				if(isdigit(ch)) { // takes lexemes beginning with letters but containing numbers
					while(!(isspace(ch)) || ch != '\n') {
						lexeme[lexeme_index++] = ch;
						ch = getNextChar(file);
					}
					storeToken(token, tokens, lexeme, INVALID);
					break;
				}
				
				lexeme[lexeme_index] = '\0';
				column--;
				ungetc(ch, file);
				
				if(isKeyword(lexeme, ch, &type, file)) {
		            storeToken(token, tokens, lexeme, type);
		    	} else if(isReservedWord(lexeme, ch, &type, file)) {
		            storeToken(token, tokens, lexeme, type);
				} else if(isNoiseWord(lexeme, ch, &type, file)){
					type = NOISE_WORD;
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
		        } else if(isDelimiter(lexeme, ch, &type, file)) {
					storeToken(token, tokens, lexeme, type);

                    if(type == DBL_QUOTE) {
                        // will try to change to switch case implementation later
                        
                        lexeme_index = 0;
                        ch = getNextChar(file);
                        
                        while(ch != '"' ) {
                            lexeme[lexeme_index++] = ch; 
                            ch = getNextChar(file);

                            if(ch == '"') {
                                Token *token = malloc(sizeof(Token));
                                tokens_size++;
                                storeToken(token, tokens, lexeme, STRING);
                            }
                        }

                        Token *token = malloc(sizeof(Token)); // allocates memory for current token
	                    tokens_size++;
                        lexeme_index = 0;
                        lexeme[lexeme_index++] = ch;
                        storeToken(token, tokens, lexeme, DBL_QUOTE);
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
    } // end of while(ch != EOF)

    tokens[tokens_index].value = '\0'; // appends null terminator to indicate the end of tokens
    tokens[tokens_index].type = END_OF_TOKENS;
    free(lexeme);
    return tokens;
}

int isNumLiteral(char *lexeme, char ch, int *type, FILE *file) {
	int has_decimal = 0;
/* no handling for strings starting with numbers
   but ending in letters yet eg. (123sd, 422d)
   
   current output:
   123 - INT
   sd - INVALID
*/	
    
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
	column--;
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
            	column--;
            	*type = INVALID;
                return 0; // invalid variable
            }
        case 2: // checks if next character is valid
        	//if(isalpha(ch) || isdigit(ch)) {
				while(isalpha(ch) || isdigit(ch)) {
	            	lexeme[lexeme_index++] = ch;
	                ch = getNextChar(file);
	            }
	      /*  } else {																				> supposed to take entire invalid identifier
	            while(!(isspace(ch) || ch == '\n' || ch == '\t')) { // take entire invalid string	> like #name$var will output "#name$var - INVALID"
            		lexeme[lexeme_index++] = ch;													> issue is that expressions will be INVALID
                	ch = getNextChar(file);															> ex. (#b + #a)  --->  #a) - INVALID
                	return 0;																		
                }																					> current behavior:
            }*/																					  //> #name$var ---> #name - IDENT, $ - INVALID, var - INVALID
            
            ungetc(ch, file);
            column--;
            if(lexeme[0] == '#') {
            	*type = VAR_IDENT;
			} else {
				*type = FUNC_IDENT;
			}
           	return 1; // invalid character
        default:
                return 0; // Invalid variable
    }
}

int isKeyword(char *lexeme, char ch, int *type, FILE *file) {
	int i = 0;  // Index for lexeme
    ch = lexeme[i];  // Start by checking the first character

    switch(ch) {
        case 'b':  // Start with 'b' for the keyword 'beegin'
            if (i + 5 < strlen(lexeme) &&
                lexeme[i + 1] == 'e' && 
                lexeme[i + 2] == 'e' && 
                lexeme[i + 3] == 'g' && 
                lexeme[i + 4] == 'i' && 
                lexeme[i + 5] == 'n' &&
                lexeme[i + 6] == '\0') {
                *type = BEEGIN_TOKEN;
                return 1;  // Matched 'beegin'
            }
            else if (i + 6 < strlen(lexeme) &&
                lexeme[i + 1] == 'e' && 
                lexeme[i + 2] == 'e' && 
                lexeme[i + 3] == 'g' && 
                lexeme[i + 4] == 'o' &&
                lexeme[i + 5] == 'n' &&
                lexeme[i + 6] == 'e'&&
                lexeme[i + 7] == '\0'){
                *type = BEEGONE_TOKEN;
                return 1; //matched 'beegone'
            }
            else if (i + 6 < strlen(lexeme) &&
                lexeme[i + 1] == 'u' && 
                lexeme[i + 2] == 'z' && 
                lexeme[i + 3] == 'z' && 
                lexeme[i + 4] == 'o' &&
                lexeme[i + 5] == 'u' &&
                lexeme[i + 6] == 't'&&
                lexeme[i + 7] == '\0') {
                *type = BUZZOUT_TOKEN;
            	return 1;  // Matched 'buzzout'
            }
            else if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'u' && 
                lexeme[i + 2] == 'z' && 
                lexeme[i + 3] == 'z'&&
                lexeme[i + 4] == '\0') {
                *type = BUZZ_TOKEN;
                return 1;  // Matched 'buzz'
            }
            break;

        case 'c':  // Start with 'c' for the keyword 'case'
            if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'a' && 
                lexeme[i + 2] == 's' && 
                lexeme[i + 3] == 'e'&&
                lexeme[i + 4] == '\0') {
                *type = CASE_TOKEN;
                return 1;  // Matched 'case'
            }
            break;

        case 'd':  // Start with 'd' for the keyword 'do' or 'downto'
            if (i + 5 < strlen(lexeme) &&
                lexeme[i + 1] == 'o' && 
                lexeme[i + 2] == 'w' && 
                lexeme[i + 3] == 'n' && 
                lexeme[i + 4] == 't' && 
                lexeme[i + 5] == 'o'&&
                lexeme[i + 6] == '\0') {
                *type = DOWNTO_TOKEN;
                return 1;  // Matched 'downto'
            } 
            else if (i + 1 < strlen(lexeme) && 
                lexeme[i + 1] == 'o'&&
                lexeme[i + 2] == '\0') {
                *type = DO_TOKEN;
                return 1;  // Matched 'do'
            }
            break;

        case 'e':  // Start with 'e' for the keyword 'else' or 'elseif'
            if (i + 5 < strlen(lexeme) &&
                lexeme[i + 1] == 'l' && 
                lexeme[i + 2] == 's' && 
                lexeme[i + 3] == 'e' &&
                lexeme[i + 4] == 'i' && 
                lexeme[i + 5] == 'f'&&
                lexeme[i + 6] == '\0') {
                *type = ELSEIF_TOKEN;
                return 1;  // Matched 'elseif'
            } 
            else if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'l' && 
                lexeme[i + 2] == 's' && 
                lexeme[i + 3] == 'e'&&
                lexeme[i + 4] == '\0') {
                *type = ELSE_TOKEN;
                return 1;  // Matched 'else'
            }
            break;

        case 'f':  // Start with 'f' for the keyword 'for'
            if (i + 2 < strlen(lexeme) &&
                lexeme[i + 1] == 'o' && 
                lexeme[i + 2] == 'r'&&
                lexeme[i + 3] == '\0') {
                *type = FOR_TOKEN;
                return 1;  // Matched 'for'
            }
            break;

        case 'g':  // Start with 'g' for the keyword 'gather'
            if (i + 5 < strlen(lexeme) &&
                lexeme[i + 1] == 'a' && 
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
            if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'i' && 
                lexeme[i + 2] == 'v' && 
                lexeme[i + 3] == 'e'&&
                lexeme[i + 4] == '\0') {
                *type = HIVE_TOKEN;
                return 1;  // Matched 'hive'
           }
            else if (i + 4 < strlen (lexeme) &&
                lexeme[i + 1] == 'o' &&
                lexeme[i + 2] == 'v' &&
                lexeme[i + 3] == 'e' &&
                lexeme[i + 4] == 'r' &&
                lexeme[i + 5] == '\0') {
                *type = HOVER_TOKEN;
                return 1; //matched hover
            }
            break;

        case 'i':  // Start with 'i' for the keyword 'if' or 'is'
            if (i + 1 < strlen(lexeme) && 
                lexeme[i + 1] == 'f' &&
                lexeme[i + 2] == '\0') {
                *type = IF_TOKEN;
                return 1;  // Matched 'if'
            }
            else if (i + 1 < strlen(lexeme) && 
                lexeme[i + 1] == 's' &&
                lexeme[i + 2] == '\0') {
                *type = IS_TOKEN;
                return 1;  // Matched 'is'
            }
            break;

        case 'q':  // Start with 'q' for the keyword 'queenbee'
            if (i + 7 < strlen(lexeme) &&
                lexeme[i + 1] == 'u' && 
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

        case 'r':  // Start with 'r' for the keyword 'returns'
            if (i + 6 < strlen(lexeme) &&
                lexeme[i + 1] == 'e' && 
                lexeme[i + 2] == 't' && 
                lexeme[i + 3] == 'u' && 
                lexeme[i + 4] == 'r' && 
                lexeme[i + 5] == 'n' && 
                lexeme[i + 6] == 's' &&
                lexeme[i + 7] == '\0') {
                *type = RETURNS_TOKEN;
                return 1;  // Matched 'returns'
            }
            break;

        case 's':  // Start with 's' for the keyword 'size', 'sting', 'switch'
            if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'i' && 
                lexeme[i + 2] == 'z' && 
                lexeme[i + 3] == 'e' &&
                lexeme[i + 4] == '\0') {
                *type = SIZE_TOKEN;
                return 1;  // Matched 'size'
            }
            else if (i + 4 < strlen(lexeme) &&
                lexeme[i + 1] == 't' && 
                lexeme[i + 2] == 'i' && 
                lexeme[i + 3] == 'n' && 
                lexeme[i + 4] == 'g' &&
                lexeme[i + 5] == '\0') {
                *type = STING_TOKEN;
                return 1;  // Matched 'sting'
            }
            else if (i + 5 < strlen(lexeme) &&
                lexeme[i + 1] == 'w' && 
                lexeme[i + 2] == 'i' && 
                lexeme[i + 3] == 't' && 
                lexeme[i + 4] == 'c' && 
                lexeme[i + 5] == 'h' &&
                lexeme[i + 6] == '\0') {
                *type = SWITCH_TOKEN;
                return 1;  // Matched 'switch'
            }
            break;

        case 't':  // Start with 't' for the keyword 'this'
            if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'h' && 
                lexeme[i + 2] == 'i' && 
                lexeme[i + 3] == 's' &&
                lexeme[i + 4] == '\0') {
                *type = THIS_TOKEN;
                return 1;  // Matched 'this'
            }
            break;

        case 'u':  // Start with 'u' for the keyword 'upto'
            if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'p' && 
                lexeme[i + 2] == 't' && 
                lexeme[i + 3] == 'o' &&
                lexeme[i + 4] == '\0') {
                *type = UPTO_TOKEN;
                return 1;  // Matched 'upto'
            }
            break;

        case 'w':  // Start with 'w' for the keyword 'while'
            if (i + 4 < strlen(lexeme) &&
                lexeme[i + 1] == 'h' && 
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

int isReservedWord(char *lexeme, char ch, int *type, FILE *file) {
	int i = 0;  // Index for lexeme
    ch = lexeme[i];  // Start by checking the first character

	switch(ch){
        case 'b':
            if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'o' &&
                lexeme[i + 2] == 'o' &&
                lexeme[i + 3] == 'l' &&
                lexeme[i + 4] == '\0') {
                *type = BOOL_TOKEN;
                return 1; //matched bool
            }
            break;

        case 'c':
            if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'h' &&
                lexeme[i + 2] == 'a' &&
                lexeme[i + 3] == 'r' &&
                lexeme[i + 4] == '\0') {
                *type = CHAR_TOKEN;
                return 1; //matched char
            }
            else if (i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'h' &&
                lexeme[i + 2] == 'a' &&
                lexeme[i + 3] == 'i' &&
                lexeme[i + 4] == 'n' &&
                lexeme[i + 5] == '\0' ) {
                *type = CHAIN_TOKEN;
                return 1; //matched chain
                }
            break;
            
        case 'i':
            if (i + 2 < strlen(lexeme) &&
                lexeme[i + 1] == 'n' &&
                lexeme[i + 2] == 't' &&
                lexeme[i + 3] == '\0') {
                *type = INT_TOKEN;
                return 1; //matched int
            }
            break;

        case 'f':
            if (i + 4 < strlen(lexeme) &&
                lexeme[i + 1] == 'l' &&
                lexeme[i + 2] == 'o' &&
                lexeme[i + 3] == 'a' &&
                lexeme[i + 4] == 't' &&
                lexeme[i + 5] == '\0') {
                *type = FLOAT_TOKEN;
                return 1; //matched float
            }
            else if (i + 4 < strlen(lexeme) &&
                lexeme[i + 1] == 'a' &&
                lexeme[i + 2] == 'l' &&
                lexeme[i + 3] == 's' &&
                lexeme[i + 4] == 'e' &&
                lexeme[i + 5] == '\0') {
                *type = FALSE_TOKEN;
                return 1;
                }
            break;

        case 't':
            if(i + 3 < strlen(lexeme) &&
                lexeme[i + 1] == 'r' &&
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

int isNoiseWord(char *lexeme, char ch, int *type, FILE *file) {
	int i = 0;  // Index for lexeme
    ch = lexeme[i];  // Start by checking the first character

    switch(ch){
        case 'r':
            if (i + 10 < strlen(lexeme) &&
                lexeme[i + 1] == 'e' &&
                lexeme[i + 2] == 't' &&
                lexeme[i + 3] == 'u' &&
                lexeme[i + 4] == 'r' &&
                lexeme[i + 5] == 'n' &&
                lexeme[i + 6] == 'v' &&
                lexeme[i + 7] == 'a' &&
                lexeme[i + 8] == 'l' &&
                lexeme[i + 9] == 'u' &&
                lexeme[i + 10] == 'e' &&
                lexeme[i + 11] == '\0') {
                return 1; //matched returnvalue
            }
            break;
        }
        return 0;
}

int isDelimiter(char *lexeme, char ch, int *type, FILE *file) {
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
        default:
            return 0; // not a delimiter
    }
}

int isOperator(char *lexeme, char ch, int *type, FILE *file) {
	switch(ch) {
    	case '+':	
    		*type = ADDITION;
    		return 1;
    	case '-':
    		*type = SUBTRACTION;
    		return 1;
    	case '*':
    		*type = MULTIPLICATION;
    		return 1;
    	case '/':
    		*type = DIVISION;
    		return 1;
    	case '%':
    		*type = MODULO;
    		return 1;
    	case '^':
    		*type = EXPONENT;
    		return 1;
    	case 'D':
    		ch = getc(file);
    		if(ch == 'I') {
    			lexeme[lexeme_index++] = ch;
    			ch = getc(file);
    			if(ch == 'V') {
    				lexeme[lexeme_index++] = ch;
    				*type = INT_DIVISION;
    				return 1;
				} else {
					ungetc(ch, file);
					return 0;
				}
			} else {
				ungetc(ch, file);
				return 0;
			}
		case '>':
    		ch = getNextChar(file);
    		if(ch == '=') {
    			lexeme[lexeme_index++] = ch;
    			*type = GREATER_EQUAL;
    			return 1;
			} else {
				ungetc(ch, file);
				column--;
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
				column--;
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
				column--;
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
				column--;
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
				column--;
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
				column--;
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
	column++;
	
	// seperate characters by class
	if(ch == '<') {
		temp = getc(file);
        if(temp == '|') {
            char_class = COMMENT_CLASS;
            ungetc(temp, file);
        } else {
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
    		column = 0;
		}
		ch = getNextChar(file);
	}
	
	return ch;
}

void storeToken(Token *token, Token *tokens, char *lexeme, int type) {
    token->value = malloc(strlen(lexeme) + 1);
    lexeme[lexeme_index] = '\0';
    strcpy(token->value, lexeme);
    token->line = line;
    token->column = column;
    token->type = type;
    
    tokens[tokens_index] = *token;
    tokens_index++;
}
