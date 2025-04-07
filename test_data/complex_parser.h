#ifndef COMPLEX_PARSER_H
#define COMPLEX_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Token types */
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_EQUALS,
    TOKEN_SEMICOLON,
    TOKEN_EOF
} TokenType;

/* Token structure */
typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
} Token;

/* AST node types */
typedef enum {
    AST_BINARY_OP,
    AST_NUMBER,
    AST_VARIABLE,
    AST_ASSIGNMENT
} ASTNodeType;

/* Binary operation types */
typedef enum {
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE
} BinaryOpType;

/* Forward declaration for AST node */
typedef struct ASTNode ASTNode;

/* AST binary operation node */
typedef struct {
    BinaryOpType op;
    ASTNode* left;
    ASTNode* right;
} BinaryOpNode;

/* AST assignment node */
typedef struct {
    char* variable_name;
    ASTNode* value;
} AssignmentNode;

/* Generic AST node */
struct ASTNode {
    ASTNodeType type;
    union {
        double number_value;
        char* variable_name;
        BinaryOpNode binary_op;
        AssignmentNode assignment;
    };
};

/* Lexer structure */
typedef struct {
    char* input;
    int position;
    int line;
    int column;
} Lexer;

/* Parser structure */
typedef struct {
    Lexer lexer;
    Token current_token;
} Parser;

/* Variable table entry */
typedef struct {
    char* name;
    double value;
} Variable;

/* Interpreter structure */
typedef struct {
    Variable* variables;
    int variable_count;
    int variable_capacity;
} Interpreter;

/* Lexer functions */
Lexer* lexer_init(const char* input);
void lexer_free(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);

/* Parser functions */
Parser* parser_init(const char* input);
void parser_free(Parser* parser);
ASTNode* parser_parse(Parser* parser);
ASTNode* parser_parse_expression(Parser* parser);
ASTNode* parser_parse_assignment(Parser* parser);

/* AST functions */
void ast_free(ASTNode* node);
void ast_print(ASTNode* node, int indent);

/* Interpreter functions */
Interpreter* interpreter_init();
void interpreter_free(Interpreter* interpreter);
double interpreter_evaluate(Interpreter* interpreter, ASTNode* node);
void interpreter_set_variable(Interpreter* interpreter, const char* name, double value);
double interpreter_get_variable(Interpreter* interpreter, const char* name);

/* Main parsing function */
double parse_and_evaluate(const char* input);

#endif /* COMPLEX_PARSER_H */