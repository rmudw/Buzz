#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "lex.h"
#include "parser.h"

const char* VALID_EXTENSION = ".bz";

void writeToken(Token *tokens, FILE *outputFile) {
    printf("| %-10s | %-25s | %-30s |\n", "LINE", "LEXEME", "TOKEN TYPE");
	fprintf(outputFile, "| %-10s | %-25s | %-30s |\n", "LINE", "LEXEME", "TOKEN TYPE");

    char *token_type[] = {
		"ADDITION", "SUBTRACTION", "MULTIPLICATION", "DIVISION", 
		"MODULO", "EXPONENT", "INT_DIVISION", "ASSIGNMENT_OP",

		"GREATER_THAN", "LESS_THAN", "IS_EQUAL_TO",
		"GREATER_EQUAL", "LESS_EQUAL", "NOT_EQUAL",

		"AND", "OR", "NOT", "INCREMENT", "DECREMENT",

		"SEMICOLON", "COMMA", "LEFT_PAREN", "RIGHT_PAREN", "LEFT_BRACKET",
		"RIGHT_BRACKET", "LEFT_BRACE", "RIGHT_BRACE", "DBL_QUOTE", "SNGL_QUOTE", "COLON",

		"BUZZ_TOKEN", "BEEGIN_TOKEN", "QUEENBEE_TOKEN", "BEEGONE_TOKEN", "FOR_TOKEN", "WHILE_TOKEN", "DO_TOKEN", 
		"HIVE_TOKEN", "STING_TOKEN", "IF_TOKEN", "RETURN_TOKEN", "ELSEIF_TOKEN", "ELSE_TOKEN", 
		"HOVER_TOKEN", "GATHER_TOKEN", "BUZZOUT_TOKEN", "SWITCH_TOKEN", "CASE_TOKEN", "DEFAULT_TOKEN",

		"CHAR_TOKEN", "CHAIN_TOKEN", "INT_TOKEN", "FLOAT_TOKEN", "BOOL_TOKEN", "TRUE_TOKEN", "FALSE_TOKEN",

		"INTEGER", "FLOAT", "STRING", "CHARACTER",

		"COMMENT_BEGIN", "COMMENT", "COMMENT_END", "VAR_IDENT", "FUNC_IDENT", "NOISE_WORD", "INVALID", "END_OF_TOKENS"
	};

    int i = 0;
    while(tokens[i].type != END_OF_TOKENS) {
        printf("| %-10d | %-25s | %-30s |\n", tokens[i].line, tokens[i].value, token_type[tokens[i].type]);
        fprintf(outputFile, "| %-10d | %-25s | %-30s |\n", tokens[i].line, tokens[i].value, token_type[tokens[i].type]);
        i++; 
    }
}

// Function to check if the file extension is correct
void check_file_type(const char* filename, const char* expectedExtension) {
    const char *dot = strrchr(filename, '.');
    if (!dot || strcmp(dot, expectedExtension) != 0) {
        fprintf(stderr, "Error: Invalid file type '%s'. Expected '%s'.\n", filename, expectedExtension);
        exit(EXIT_FAILURE);
    }
}

int main() {
    const char *fileName = "test.bz";         // Hardcoded input file name
    const char *outputName = "SymbolTable.bz"; // Hardcoded output file name

    // Check extensions
    check_file_type(fileName, VALID_EXTENSION);
    check_file_type(outputName, VALID_EXTENSION);

    // Open input file
    FILE *file = fopen(fileName, "r");
    if (!file) {
        perror("Error: Unable to open input file");
        return EXIT_FAILURE;
    }

    // Debug: Check input file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size == 0) {
        fprintf(stderr, "Error: Input file is empty.\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Open output file
    FILE *outputFile = fopen(outputName, "w");
    if (!outputFile) {
        perror("Error: Unable to create output file");
        fclose(file); // Close input file
        return EXIT_FAILURE;
    }

	Token *tokens = lex(file);
    fclose(file);

    writeToken(tokens, outputFile);
	fclose(outputFile);

	Node *AST = parseProgram(tokens);
	return 0;
}
