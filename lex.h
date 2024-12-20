#ifndef LEX_H
#define LEX_H

#include <stdio.h>

typedef enum {
	// arithmetic operators
	ADDITION,	//0
	SUBTRACTION,
	MULTIPLICATION,
	DIVISION,
	MODULO,
	EXPONENT,	//5
	INT_DIVISION,
	ASSIGNMENT_OP,
	
	// relational operators
	GREATER_THAN,
	LESS_THAN,
	IS_EQUAL_TO, //10
	GREATER_EQUAL,
	LESS_EQUAL,
	NOT_EQUAL,
	
	// logical operators
	AND,
	OR,			//15
	NOT,
	
	// delimiters
	SEMICOLON,
	COMMA,
	LEFT_PAREN,
	RIGHT_PAREN, //20
	LEFT_BRACKET,
	RIGHT_BRACKET,
	LEFT_BRACE,
	RIGHT_BRACE,
	DBL_QUOTE,	//25
	SNGL_QUOTE,
	
	INT,
	FLOAT,
	STRING,
	
	COMMENT,
	VAR_IDENT,
	FUNC_IDENT,
	KEYWORD,
	RESERVED_WORD,
	NOISE_WORD,
	INVALID,
	END_OF_TOKENS
} TokenType;

typedef struct {
	TokenType type;
	char *value;
	unsigned int line;
	unsigned int column;
} Token;

Token *lex(FILE *file);
int isNumLiteral(char *lexeme, char ch, int *type, FILE *file);
int isKeyword(char *lexeme, char ch, int *type, FILE *file);
int isReservedWord(char *lexeme, char ch, int *type, FILE *file);
int isNoiseWord(char *lexeme, char ch, int *type, FILE *file);
int isIdentifier(char *lexeme, char ch, int *type, FILE *file);
int isDelimiter(char *lexeme, char ch, int *type, FILE *file);
int isOperator(char *lexeme, char ch, int *type, FILE *file);
char getNextChar(FILE *file);
char getNonBlank(FILE *file);
void storeToken(Token *token, Token *tokens, char *lexeme, int type);

#endif
