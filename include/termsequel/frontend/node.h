#ifndef TERMSEQUEL_NODE_H
#define TERMSEQUEL_NODE_H

#include "cgeneric/array.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * Statements
 */
typedef struct ast_statement_node ast_statement_node;
typedef struct ast_select_statement_node ast_select_statement_node;

typedef struct ast_column_node ast_column_node;
typedef struct ast_order_node ast_order_node;
typedef struct ast_limit_node ast_limit_node;
typedef struct ast_group_node ast_group_node;

/**
 * Expressions Nodes
 */
typedef struct ast_expression_node ast_expression_node;
typedef struct ast_expression_list_node ast_expression_list_node;
typedef struct ast_binary_expression_node ast_binary_expression_node;
typedef struct ast_unary_expression_node ast_unary_expression_node;
typedef struct ast_literal_expression_node ast_literal_expression_node;
typedef struct ast_literal_identifier_node ast_literal_identifier_node;
typedef struct ast_function_expression ast_function_expression;

typedef enum ast_statement_type
{
    STATEMENT_UNKNOWN = 0,
    STATEMENT_SELECT,
} ast_statement_type;

typedef enum ast_expression_type
{
    EXPRESSION_INVALID = 0,
    EXPRESSION_LITERAL,
    EXPRESSION_UNARY,
    EXPRESSION_BINARY,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_FUNCTION,
    EXPRESSION_LIST,
} ast_expression_type;

typedef enum ast_operator_type
{
    OPERATOR_INVALID = 0,
    OPERATOR_PLUS,
    OPERATOR_MINUS,
    OPERATOR_MULTIPLY,
    OPERATOR_DIVIDE,
    OPERATOR_MOD,
    OPERATOR_AND,
    OPERATOR_OR,
    OPERATOR_EQUAL,
    OPERATOR_NOT_EQUAL,
    OPERATOR_LESS,
    OPERATOR_LESS_THAN_OR_EQUAL,
    OPERATOR_GREATER,
    OPERATOR_GREATER_THAN_OR_EQUAL,
    OPERATOR_LIKE,
    OPERATOR_NOT,
} ast_operator_type;

extern const char* OPERATOR_NAMES[];

typedef enum ast_literal_type
{
    LITERAL_TYPE_INTEGER = 0,
    LITERAL_TYPE_REAL,
    LITERAL_TYPE_STRING,
    LITERAL_TYPE_IDENTIFIER,
    LITERAL_TYPE_BOOLEAN,
    LITERAL_TYPE_DATE,
} ast_literal_type;

typedef enum ast_function_type
{
    FUNCTION_TYPE_UPPER = 0,
    FUNCTION_TYPE_LOWER,
    FUNCTION_TYPE_MAX,
    FUNCTION_TYPE_MIN,
    FUNCTION_TYPE_AVG,
    FUNCTION_TYPE_DATE,
} ast_function_type;

typedef enum ast_node_kind
{
    KIND_INVALID = 0 << 0,
    KIND_INTEGER = 1 << 1,
    KIND_REAL = 1 << 2,
    KIND_STRING = 1 << 3,
    KIND_BOOLEAN = 1 << 4,
    KIND_DATE = 1 << 5,
    // accepts integer or real
    KIND_ANY_NUMBER = KIND_INTEGER | KIND_REAL,
} ast_node_kind;

typedef enum ast_order_mode
{
    ORDER_ASC = 0,
    ORDER_DESC
} ast_order_mode;

struct ast_statement_node
{
    ast_statement_type type;
    union
    {
        ast_select_statement_node* select;
    };
};

struct ast_column_node
{
    ast_expression_node* expression;
    char name[ 40 ];
};

struct ast_select_statement_node
{
    array* columns;
    ast_expression_node* filter;
    ast_order_node* sort;
    ast_limit_node* limit;
    ast_group_node* group;
    char source[ 40 ];
};

struct ast_expression_node
{
    ast_expression_type type;
    ast_node_kind kind;
    union
    {
        ast_binary_expression_node* binary;
        ast_unary_expression_node* unary;
        ast_literal_expression_node* literal;
        ast_function_expression* function;
        ast_expression_list_node* list;
    };
};

struct ast_expression_list_node
{
    uint8_t size;
    array* expressions;
};

struct ast_binary_expression_node
{
    ast_operator_type operator;
    ast_expression_node* left_expression;
    ast_expression_node* right_expression;
};

struct ast_unary_expression_node
{
    ast_operator_type operator;
    ast_expression_node* expression;
};

struct ast_literal_expression_node
{
    ast_literal_type type;
    union
    {
        long integer;
        double real;
        char* string;
        ast_literal_identifier_node* identifier;
        bool boolean;
    };
};

struct ast_literal_identifier_node
{
    // the real value that was extracted from OS
    ast_literal_expression_node* real_value;

    // name like: FILENAME, SIZE, DATE
    char name[];
};

struct ast_function_expression
{
    ast_function_type type;
    ast_expression_node* expression;
};

struct ast_order_node
{
    uint8_t column_number;
    ast_order_mode mode;
    ast_order_node* next;
};

struct ast_limit_node
{
    uint16_t value;
};

struct ast_group_node
{
};

#endif // TERMSEQUEL_NODE_H