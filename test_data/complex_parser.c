#include "complex_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Helper functions */
int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int is_alphanumeric(char c) {
    return is_alpha(c) || is_digit(c);
}

/* Lexer implementation */
Lexer* lexer_init(const char* input) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->input = strdup(input);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    return lexer;
}

void lexer_free(Lexer* lexer) {
    free(lexer->input);
    free(lexer);
}

void lexer_advance(Lexer* lexer) {
    if (lexer->input[lexer->position] == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    lexer->position++;
}

char lexer_peek(Lexer* lexer) {
    return lexer->input[lexer->position];
}

void lexer_skip_whitespace(Lexer* lexer) {
    while (is_whitespace(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
}

Token lexer_next_token(Lexer* lexer) {
    lexer_skip_whitespace(lexer);
    
    Token token;
    token.line = lexer->line;
    token.column = lexer->column;
    token.value = NULL;
    
    char current = lexer_peek(lexer);
    
    // End of input
    if (current == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }
    
    // Identifiers
    if (is_alpha(current)) {
        int start = lexer->position;
        while (is_alphanumeric(lexer_peek(lexer))) {
            lexer_advance(lexer);
        }
        int length = lexer->position - start;
        
        token.type = TOKEN_IDENTIFIER;
        token.value = (char*)malloc(length + 1);
        strncpy(token.value, lexer->input + start, length);
        token.value[length] = '\0';
        
        return token;
    }
    
    // Numbers
    if (is_digit(current) || current == '.') {
        int start = lexer->position;
        int dot_count = 0;
        
        if (current == '.') {
            dot_count++;
            lexer_advance(lexer);
        }
        
        while (is_digit(lexer_peek(lexer)) || lexer_peek(lexer) == '.') {
            if (lexer_peek(lexer) == '.') {
                dot_count++;
                if (dot_count > 1) {
                    break;
                }
            }
            lexer_advance(lexer);
        }
        
        int length = lexer->position - start;
        
        token.type = TOKEN_NUMBER;
        token.value = (char*)malloc(length + 1);
        strncpy(token.value, lexer->input + start, length);
        token.value[length] = '\0';
        
        return token;
    }
    
    // Operators and other symbols
    switch (current) {
        case '+':
            token.type = TOKEN_PLUS;
            break;
        case '-':
            token.type = TOKEN_MINUS;
            break;
        case '*':
            token.type = TOKEN_MULTIPLY;
            break;
        case '/':
            token.type = TOKEN_DIVIDE;
            break;
        case '(':
            token.type = TOKEN_LPAREN;
            break;
        case ')':
            token.type = TOKEN_RPAREN;
            break;
        case '=':
            token.type = TOKEN_EQUALS;
            break;
        case ';':
            token.type = TOKEN_SEMICOLON;
            break;
        default:
            // Invalid character
            fprintf(stderr, "Error: Unexpected character '%c' at line %d, column %d\n", 
                    current, lexer->line, lexer->column);
            lexer_advance(lexer);
            return lexer_next_token(lexer);
    }
    
    lexer_advance(lexer);
    return token;
}

/* AST functions */
ASTNode* ast_create_number(double value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_NUMBER;
    node->number_value = value;
    return node;
}

ASTNode* ast_create_variable(char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE;
    node->variable_name = strdup(name);
    return node;
}

ASTNode* ast_create_binary_op(BinaryOpType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_BINARY_OP;
    node->binary_op.op = op;
    node->binary_op.left = left;
    node->binary_op.right = right;
    return node;
}

ASTNode* ast_create_assignment(char* variable_name, ASTNode* value) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = AST_ASSIGNMENT;
    node->assignment.variable_name = strdup(variable_name);
    node->assignment.value = value;
    return node;
}

void ast_free(ASTNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case AST_BINARY_OP:
            ast_free(node->binary_op.left);
            ast_free(node->binary_op.right);
            break;
        case AST_VARIABLE:
            free(node->variable_name);
            break;
        case AST_ASSIGNMENT:
            free(node->assignment.variable_name);
            ast_free(node->assignment.value);
            break;
        case AST_NUMBER:
            // Nothing to free
            break;
    }
    
    free(node);
}

void ast_print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void ast_print(ASTNode* node, int indent) {
    if (!node) return;
    
    switch (node->type) {
        case AST_NUMBER:
            ast_print_indent(indent);
            printf("Number: %f\n", node->number_value);
            break;
        case AST_VARIABLE:
            ast_print_indent(indent);
            printf("Variable: %s\n", node->variable_name);
            break;
        case AST_BINARY_OP:
            ast_print_indent(indent);
            printf("BinaryOp: ");
            switch (node->binary_op.op) {
                case OP_ADD: printf("+"); break;
                case OP_SUBTRACT: printf("-"); break;
                case OP_MULTIPLY: printf("*"); break;
                case OP_DIVIDE: printf("/"); break;
            }
            printf("\n");
            ast_print(node->binary_op.left, indent + 1);
            ast_print(node->binary_op.right, indent + 1);
            break;
        case AST_ASSIGNMENT:
            ast_print_indent(indent);
            printf("Assignment: %s\n", node->assignment.variable_name);
            ast_print(node->assignment.value, indent + 1);
            break;
    }
}

/* Parser implementation */
Parser* parser_init(const char* input) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    parser->lexer = *lexer_init(input);
    parser->current_token = lexer_next_token(&parser->lexer);
    return parser;
}

void parser_free(Parser* parser) {
    if (parser->current_token.value) {
        free(parser->current_token.value);
    }
    lexer_free(&parser->lexer);
    free(parser);
}

void parser_eat(Parser* parser, TokenType token_type) {
    if (parser->current_token.type == token_type) {
        if (parser->current_token.value) {
            free(parser->current_token.value);
        }
        parser->current_token = lexer_next_token(&parser->lexer);
    } else {
        fprintf(stderr, "Parser error: Expected token type %d, got %d at line %d, column %d\n", 
                token_type, parser->current_token.type, 
                parser->current_token.line, parser->current_token.column);
        exit(1);
    }
}

ASTNode* parser_parse_factor(Parser* parser) {
    Token token = parser->current_token;
    ASTNode* node = NULL;
    
    switch (token.type) {
        case TOKEN_NUMBER:
            parser_eat(parser, TOKEN_NUMBER);
            node = ast_create_number(atof(token.value));
            break;
        case TOKEN_IDENTIFIER:
            parser_eat(parser, TOKEN_IDENTIFIER);
            node = ast_create_variable(token.value);
            break;
        case TOKEN_LPAREN:
            parser_eat(parser, TOKEN_LPAREN);
            node = parser_parse_expression(parser);
            parser_eat(parser, TOKEN_RPAREN);
            break;
        case TOKEN_PLUS:
            parser_eat(parser, TOKEN_PLUS);
            node = parser_parse_factor(parser);
            break;
        case TOKEN_MINUS:
            parser_eat(parser, TOKEN_MINUS);
            node = ast_create_binary_op(OP_SUBTRACT, ast_create_number(0), parser_parse_factor(parser));
            break;
        default:
            fprintf(stderr, "Parser error: Unexpected token type %d at line %d, column %d\n", 
                    token.type, token.line, token.column);
            exit(1);
    }
    
    return node;
}

ASTNode* parser_parse_term(Parser* parser) {
    ASTNode* node = parser_parse_factor(parser);
    
    while (parser->current_token.type == TOKEN_MULTIPLY || 
           parser->current_token.type == TOKEN_DIVIDE) {
        TokenType token_type = parser->current_token.type;
        parser_eat(parser, token_type);
        
        if (token_type == TOKEN_MULTIPLY) {
            node = ast_create_binary_op(OP_MULTIPLY, node, parser_parse_factor(parser));
        } else { // TOKEN_DIVIDE
            node = ast_create_binary_op(OP_DIVIDE, node, parser_parse_factor(parser));
        }
    }
    
    return node;
}

ASTNode* parser_parse_expression(Parser* parser) {
    ASTNode* node = parser_parse_term(parser);
    
    while (parser->current_token.type == TOKEN_PLUS || 
           parser->current_token.type == TOKEN_MINUS) {
        TokenType token_type = parser->current_token.type;
        parser_eat(parser, token_type);
        
        if (token_type == TOKEN_PLUS) {
            node = ast_create_binary_op(OP_ADD, node, parser_parse_term(parser));
        } else { // TOKEN_MINUS
            node = ast_create_binary_op(OP_SUBTRACT, node, parser_parse_term(parser));
        }
    }
    
    return node;
}

ASTNode* parser_parse_assignment(Parser* parser) {
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        return parser_parse_expression(parser);
    }
    
    char* variable_name = strdup(parser->current_token.value);
    parser_eat(parser, TOKEN_IDENTIFIER);
    
    if (parser->current_token.type != TOKEN_EQUALS) {
        ASTNode* node = ast_create_variable(variable_name);
        free(variable_name);
        return node;
    }
    
    parser_eat(parser, TOKEN_EQUALS);
    ASTNode* value = parser_parse_expression(parser);
    ASTNode* node = ast_create_assignment(variable_name, value);
    free(variable_name);
    
    return node;
}

ASTNode* parser_parse(Parser* parser) {
    ASTNode* node = parser_parse_assignment(parser);
    
    if (parser->current_token.type == TOKEN_SEMICOLON) {
        parser_eat(parser, TOKEN_SEMICOLON);
    }
    
    return node;
}

/* Interpreter implementation */
Interpreter* interpreter_init() {
    Interpreter* interpreter = (Interpreter*)malloc(sizeof(Interpreter));
    interpreter->variable_capacity = 10;
    interpreter->variable_count = 0;
    interpreter->variables = (Variable*)malloc(sizeof(Variable) * interpreter->variable_capacity);
    return interpreter;
}

void interpreter_free(Interpreter* interpreter) {
    for (int i = 0; i < interpreter->variable_count; i++) {
        free(interpreter->variables[i].name);
    }
    free(interpreter->variables);
    free(interpreter);
}

void interpreter_set_variable(Interpreter* interpreter, const char* name, double value) {
    // Check if variable already exists
    for (int i = 0; i < interpreter->variable_count; i++) {
        if (strcmp(interpreter->variables[i].name, name) == 0) {
            interpreter->variables[i].value = value;
            return;
        }
    }
    
    // If we need to resize the array
    if (interpreter->variable_count >= interpreter->variable_capacity) {
        interpreter->variable_capacity *= 2;
        interpreter->variables = (Variable*)realloc(interpreter->variables, 
                                                   sizeof(Variable) * interpreter->variable_capacity);
    }
    
    // Add new variable
    interpreter->variables[interpreter->variable_count].name = strdup(name);
    interpreter->variables[interpreter->variable_count].value = value;
    interpreter->variable_count++;
}

double interpreter_get_variable(Interpreter* interpreter, const char* name) {
    for (int i = 0; i < interpreter->variable_count; i++) {
        if (strcmp(interpreter->variables[i].name, name) == 0) {
            return interpreter->variables[i].value;
        }
    }
    
    fprintf(stderr, "Error: Undefined variable '%s'\n", name);
    return 0;
}

double interpreter_evaluate(Interpreter* interpreter, ASTNode* node) {
    if (!node) return 0;
    
    switch (node->type) {
        case AST_NUMBER:
            return node->number_value;
        case AST_VARIABLE:
            return interpreter_get_variable(interpreter, node->variable_name);
        case AST_BINARY_OP:
            double left = interpreter_evaluate(interpreter, node->binary_op.left);
            double right = interpreter_evaluate(interpreter, node->binary_op.right);
            
            switch (node->binary_op.op) {
                case OP_ADD:
                    return left + right;
                case OP_SUBTRACT:
                    return left - right;
                case OP_MULTIPLY:
                    return left * right;
                case OP_DIVIDE:
                    if (right == 0) {
                        fprintf(stderr, "Error: Division by zero\n");
                        return 0;
                    }
                    return left / right;
            }
            break;
        case AST_ASSIGNMENT:
            double value = interpreter_evaluate(interpreter, node->assignment.value);
            interpreter_set_variable(interpreter, node->assignment.variable_name, value);
            return value;
    }
    
    return 0;
}

double parse_and_evaluate(const char* input) {
    Parser* parser = parser_init(input);
    ASTNode* node = parser_parse(parser);
    
    Interpreter* interpreter = interpreter_init();
    double result = interpreter_evaluate(interpreter, node);
    
    ast_free(node);
    parser_free(parser);
    interpreter_free(interpreter);
    
    return result;
}

int main() {
    char buffer[1024];
    Interpreter* interpreter = interpreter_init();
    
    printf("Simple Expression Parser\n");
    printf("Type an expression (e.g., 2 + 3 * 4) or 'exit' to quit:\n");
    
    while (1) {
        printf("> ");
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            break;
        }
        
        if (strcmp(buffer, "exit\n") == 0 || strcmp(buffer, "quit\n") == 0) {
            break;
        }
        
        Parser* parser = parser_init(buffer);
        ASTNode* node = parser_parse(parser);
        
        printf("AST:\n");
        ast_print(node, 0);
        
        double result = interpreter_evaluate(interpreter, node);
        printf("Result: %f\n", result);
        
        ast_free(node);
        parser_free(parser);
    }
    
    interpreter_free(interpreter);
    return 0;
}