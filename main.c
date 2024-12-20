#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>

#include "lex.h"

int main(int argc, char *argv[]) {
	FILE *file = fopen("sc.bz", "r");
	FILE *outputFile = fopen("st.bz", "w");

    if (file == NULL) {
        perror("Error opening input file");
        return 1;
    }

    if (outputFile == NULL) {
        perror("Error opening output file");
        return 1;
    }

	Token *tokens = lex(file);
	
	int i = 0;
	char *token_type[] = {"ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION",
						"MODULO", "EXPONENT", "INT_DIVISION", "ASSIGNMENT_OP",
						
						// relational operators
						"GREATER_THAN", "LESS_THAN", "IS_EQUAL", "GREATER_EQUAL", 
						"LESS_EQUAL", "NOT_EQUAL",
						
						// logical operators
						"AND", "OR", "NOT",
						
						// delimiters
						"SEMICOLON", "COMMA", "LEFT_PAREN", "RIGHT_PAREN",
						"LEFT_BRACKET", "RIGHT_BRACKET", "LEFT_BRACE", "RIGHT_BRACE",
						"DBL_QUOTE", "SNGL_QUOTE",
						
						"INT", "FLOAT", "STRING",
	
						"COMMENT", "VAR_IDENT", "FUNC_IDENT", "KEYWORD", "RESERVED_WORD",
						"NOISE_WORD", "INVALID", "END"};

	printf("| %-10s | %-10s | %-25s | %-30s |\n", "LINE", "COLUMN", "LEXEME", "TOKEN TYPE");
	fprintf(outputFile, "| %-10s | %-10s | %-25s | %-30s |\n", "LINE", "COLUMN", "LEXEME", "TOKEN TYPE");

	while(tokens[i].type != END_OF_TOKENS) {
		printf("| %-10d | %-10d | %-25s | %-30s |\n", tokens[i].line, tokens[i].column, tokens[i].value, token_type[tokens[i].type]);
		fprintf(outputFile, "| %-10d | %-10d | %-25s | %-30s |\n", tokens[i].line, tokens[i].column, tokens[i].value, token_type[tokens[i].type]);
		//printf("| %-10d | %-10d | %-25s | %-30d |\n", tokens[i].line, tokens[i].column, tokens[i].value, tokens[i].type);
		i++;
	}
	
	fclose(file);
	fclose(outputFile);
	return 0;
}
