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
	char *token_type[] = {
		"ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION", 
		"MODULO", "EXPONENT", "INT_DIVISION", "ASSIGNMENT_OP",

		"GREATER_THAN", "LESS_THAN", "IS_EQUAL_TO",
		"GREATER_EQUAL", "LESS_EQUAL", "NOT_EQUAL",

		"AND", "OR", "NOT",

		"SEMICOLON", "COMMA", "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACKET",
		"RIGHT_BRACKET", "LEFT_BRACE", "RIGHT_BRACE", "DBL_QUOTE", "SNGL_QUOTE",

		"BUZZ_TOKEN", "BEEGIN_TOKEN", "QUEENBEE_TOKEN", "BEEGONE_TOKEN", "FOR_TOKEN", "THIS_TOKEN", 
		"IS_TOKEN", "WHILE_TOKEN", "DO_TOKEN", "UPTO_TOKEN", "DOWNTO_TOKEN", "HIVE_TOKEN", 
		"SIZE_TOKEN", "STING_TOKEN", "IF_TOKEN", "RETURNS_TOKEN", "ELSEIF_TOKEN", "ELSE_TOKEN", 
		"HOVER_TOKEN", "GATHER_TOKEN", "BUZZOUT_TOKEN", "SWITCH_TOKEN", "CASE_TOKEN",

		"CHAR_TOKEN", "CHAIN_TOKEN", "INT_TOKEN", "FLOAT_TOKEN", "BOOL_TOKEN", "TRUE_TOKEN", "FALSE_TOKEN",

		"INTEGER", "FLOAT", "STRING",

		"COMMENT", "VAR_IDENT", "FUNC_IDENT", "NOISE_WORD", "INVALID", "END_OF_TOKENS"
	};

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
