#include "termsequel/frontend/parser.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgeneric/array.h"
#include "termsequel/frontend/lexer.h"
#include "termsequel/frontend/node.h"
#include "termsequel/frontend/token.h"

#include "termsequel/log/logger.h"

#include "cgeneric/queue.h"
#include "cgeneric/stack.h"

typedef struct parser parser;

typedef enum operator_type
{
    OPERATOR_UNARY = 0,
    OPERATOR_BINARY,
    OPERATOR_FUNCTION,
} operator_type;

typedef struct operator operator;

typedef enum operation
{
    BINARY_OPERATION_PLUS = 0,
    BINARY_OPERATION_MINUS,
    BINARY_OPERATION_MULTIPLICATION,
    BINARY_OPERATION_DIVISION,
    UNARY_OPERATION_MINUS,
    FUNCTION_OPERATION_UPPER,
    FUNCTION_OPERATION_MAX,
    FUNCTION_OPERATION_MIN,
    OPERATOR_LEFT_PARENTHESIS,
    OPERATOR_RIGHT_PARENTHESIS,
    BINARY_OPERATION_AND,
    BINARY_OPERATION_OR,
    BINARY_LESS_THAN,
    BINARY_LESS_THAN_OR_EQUAL,
    BINARY_GREATER_THAN,
    BINARY_GREATER_THAN_OR_EQUAL,
    BINARY_EQUAL,
    BINARY_NOT_EQUAL,
    BINARY_LIKE,
    UNARY_NOT,
} operation;

struct operator
{
    operation operation;
    uint8_t precedence;
    bool left_associative;
};

struct parser
{
    lexer* lexer;
    lexeme* previous_token;
    lexeme* current_token;
};

static bool is_unary_minus ( parser* p );

static void parser_init ( parser* p, lexer* l );
static bool parser_matches_token_type ( const parser* p, token delimiter1, token delimiter2 );
static void parser_advance ( parser* p );
static void parser_free ( parser* p );

ast_select_statement_node* parse_select_statement ( parser* p );

/**
 * Expressions parsers
 */
static ast_expression_node* parse_expression_3 ( parser* p, token delimiter1, token delimiter2, token delimiter3 );
static ast_expression_node* parse_expression_2 ( parser* p, token delimiter1, token delimiter2 );
static ast_expression_node* parse_expression_1 ( parser* p, token delimiter1 );
static ast_literal_expression_node* parse_literal_expression ( parser* p );
static ast_expression_node* parse_function_expression ( parser* p );
static ast_order_node* parse_order ( parser* p );
static ast_limit_node* parse_limit ( parser* p );

static void parse_number_literal ( ast_literal_expression_node* node, const lexeme* const l );

/**
 * Handle operations,
 */
static void handle_precedence ( struct operator current, stack* operators, stack* operands );
static void handle_operation ( struct operator operator, stack * operands );
static void handle_end_of_stream ( stack* operators, stack* operands );
static operator build_operator ( const lexeme* const current, operator_type op_type );
static operator handle_relational_operator ( const lexeme* const );
static int get_type_by_operation ( operation op );

ast_statement_node* parse_statement ( const char* sql )
{
    lexer* l = lexer_init ( sql );
    parser p = { 0 };
    parser_init ( &p, l );

    ast_statement_node* root = NULL;

    if ( parser_matches_token_type ( &p, TOKEN_SELECT, TOKEN_SELECT ) )
    {
        root = malloc ( sizeof ( ast_statement_node ) );
        root->type = STATEMENT_SELECT;
        root->select = parse_select_statement ( &p );
    }
    parser_free ( &p );
    return root;
}

void parser_init ( parser* p, lexer* l )
{
    p->lexer = l;
    p->previous_token = NULL;
    p->current_token = lexer_next ( l );
}

bool parser_matches_token_type ( const parser* p, token delimiter1, token delimiter2 )
{
    return p->current_token->token_type == delimiter1 || p->current_token->token_type == delimiter2;
}

void parser_advance ( parser* p )
{
    free ( p->previous_token );
    p->previous_token = p->current_token;

    if ( lexer_has_next ( p->lexer ) )
    {
        p->current_token = lexer_next ( p->lexer );
    }
    else
    {
        p->current_token = malloc ( sizeof ( lexeme ) );
        p->current_token->token_type = TOKEN_EOF;
    }
}

void parser_free ( parser* p )
{
    free ( p->current_token );
    free ( p->previous_token );
    free ( p->lexer );
    memset ( p, 0, sizeof ( parser ) );
}

ast_select_statement_node* parse_select_statement ( parser* p )
{

    LOG_MESSAGE_DEBUG("Parsing SELECT statment ...\n");
    ast_select_statement_node* select = malloc ( sizeof ( ast_select_statement_node ) );

    // drop the select keyword
    parser_advance ( p );

    select->columns = ARRAY_CONSTRUCT ( ast_column_node, 10 );
    uint8_t columns_number = 0;

    // parse column list
    bool run = true;

    while ( run )
    {
        ast_column_node column = { 0 };
        column.expression = parse_expression_3 ( p, TOKEN_COMMA, TOKEN_FROM, TOKEN_AS );

        snprintf ( column.name, sizeof ( column.name ) / sizeof ( column.name[ 0 ] ), "COLUMN_%d", columns_number );

        if ( p->current_token->token_type == TOKEN_AS )
        {
            // drops the AS keyword
            parser_advance ( p );
            if ( p->current_token->token_type != TOKEN_STRING )
            {
                fprintf ( stderr, "SYNTAX error: Expected a string after the AS keyword\n" );
                exit ( 1 );
            }
            strncpy ( column.name, p->current_token->value, sizeof ( column.name ) / sizeof ( column.name[ 0 ] ) );
            // drops the column name
            parser_advance ( p );
        }

        if ( p->current_token->token_type == TOKEN_FROM )
        {
            parser_advance ( p );
            run = false;
        }

        else if ( p->current_token->token_type == TOKEN_COMMA )
        {
            // consume the comma or the from token
            parser_advance ( p );
        }

        ARRAY_ADD ( select->columns, columns_number++, column );
    }

    if ( p->current_token->token_type != TOKEN_STRING )
    {
        fprintf ( stderr, "SYNTAX error: Expected a string after the FROM keyword ...\n" );
        exit ( 1 );
    }

    memset ( select->source, 0, sizeof ( select->source ) / sizeof ( select->source[ 0 ] ) );
    memcpy ( select->source, p->current_token->value, strlen ( p->current_token->value ) );
    parser_advance ( p );

    ARRAY_RESIZE ( select->columns, columns_number );

    if ( p->current_token != NULL && p->current_token->token_type == TOKEN_WHERE )
    {
        LOG_MESSAGE_DEBUG("Parsing WHERE clause ...\n");
        parser_advance ( p );
        select->filter = parse_expression_2 ( p, TOKEN_EOF, TOKEN_ORDER );
    }

    select->sort = parse_order ( p );
    select->limit = parse_limit ( p );

    return select;
}

ast_expression_node* parse_expression_3 ( parser* p, token delimiter1, token delimiter2, token delimiter3 )
{
    stack* operands = STACK_CONSTRUCT ( ast_expression_node*, 100 );
    stack* operators = STACK_CONSTRUCT ( operator, 100 );

    bool run = true;

    size_t left_parenthesis_number = 0;

    while ( run )
    {
        if ( p->current_token == NULL )
        {
            // reached the end
            run = false;
            break;
        }

        LOG_MESSAGE_DEBUG("Parsing token type: \"%s\", value: \"%s\"\n", TOKEN_NAMES[p->current_token->token_type], p->current_token->value);

        if ( ( p->current_token->token_type == delimiter1 || p->current_token->token_type == delimiter2 || p->current_token->token_type == delimiter3 ) && left_parenthesis_number == 0 )
        {
            run = false;
            break;
        }

        if ( p->current_token->token_type == TOKEN_NOT )
        {
            operator op = build_operator ( p->current_token, OPERATOR_UNARY );
            STACK_PUSH ( operators, operator, op );
            parser_advance ( p );
        }
        else if ( p->current_token->token_type == TOKEN_LEFT_PARENTHESIS )
        {
            operator op;
            op.operation = OPERATOR_LEFT_PARENTHESIS;
            op.precedence = 100;
            STACK_PUSH ( operators, operator, op );
            parser_advance ( p );
            left_parenthesis_number++;
        }
        else if ( p->current_token->token_class == TOKEN_CLASS_LOGICAL_OPERATOR )
        {
            // AND, OR
            operator op;
            op.precedence = 0;
            op.left_associative = true;
            if ( p->current_token->token_type == TOKEN_AND )
            {
                op.operation = BINARY_OPERATION_AND;
            }
            else
            {
                op.operation = BINARY_OPERATION_OR;
            }
            // STACK_PUSH ( operators, operator, op );
            handle_precedence ( op, operators, operands );
            parser_advance ( p );
        }
        else if ( p->current_token->token_class == TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR )
        {
            // =, >=, > ...
            operator op = handle_relational_operator ( p->current_token );
            STACK_PUSH ( operators, operator, op );
            parser_advance ( p );
        }
        else if ( p->current_token->token_class == TOKEN_CLASS_LITERAL )
        {
            ast_expression_node* expression = malloc ( sizeof ( ast_expression_node ) );
            expression->type = EXPRESSION_LITERAL;
            expression->literal = parse_literal_expression ( p );
            STACK_PUSH ( operands, ast_expression_node*, expression );
            parser_advance ( p );
        }
        else if ( p->current_token->token_class == TOKEN_CLASS_FUNCTION )
        {
            STACK_PUSH ( operands, ast_expression_node*, parse_function_expression ( p ) );
        }
        else if ( p->current_token->token_class == TOKEN_CLASS_BINARY_ARITHMETIC_OPERATOR )
        {
            handle_precedence ( build_operator ( p->current_token, OPERATOR_BINARY ), operators, operands );
            parser_advance ( p );
        }
        else if ( p->current_token->token_type == TOKEN_MINUS )
        {
            operator ope;
            if ( is_unary_minus ( p ) )
            {
                ope = build_operator ( p->current_token, OPERATOR_UNARY );
            }
            else
            {
                ope = build_operator ( p->current_token, OPERATOR_BINARY );
            }
            STACK_PUSH ( operators, operator, ope );
            parser_advance ( p );
        }
        else if ( p->current_token->token_type == TOKEN_RIGHT_PARENTHESIS )
        {
            left_parenthesis_number--;
            while ( STACK_PEEK ( operators, operator).operation != OPERATOR_LEFT_PARENTHESIS )
            {
                handle_operation ( STACK_POP ( operators, operator), operands );
            }
            assert ( STACK_SIZE ( operators ) > 0 && "Left Paranthesis was expected!" );
            // discards the LEFT parenthesis
            STACK_POP ( operators, operator);
            parser_advance ( p );
        }
        else
        {
            fprintf ( stderr, "Unexpected token: %s near %s\n", TOKEN_NAMES[ p->current_token->token_type ], TOKEN_NAMES[ p->previous_token->token_type ] );
            exit ( 50 );
        }
    }

    handle_end_of_stream ( operators, operands );
    assert ( STACK_SIZE ( operators ) == 0 && "Operators stack must be empty at the end ..." );
    assert ( STACK_SIZE ( operands ) == 1 && "Operands Stack must have only one element at the end ..." );
    ast_expression_node* top = STACK_POP ( operands, ast_expression_node* );
    STACK_DESTROY ( operands );
    STACK_DESTROY ( operators );
    return top;
}

ast_literal_expression_node* parse_literal_expression ( parser* p )
{
    const lexeme* l = p->current_token;
    ast_literal_expression_node* node = malloc ( sizeof ( ast_literal_expression_node ) );
    if ( l->token_type == TOKEN_NUMBER )
    {
        parse_number_literal ( node, l );
    }
    else if ( l->token_type == TOKEN_STRING )
    {
        node->type = LITERAL_TYPE_STRING;
        node->string = malloc ( sizeof ( char ) * ( strlen ( l->value ) + 1 ) );
        memset ( node->string, 0, strlen ( l->value ) + 1 );
        memcpy ( node->string, l->value, strlen ( l->value ) );
    }
    else if ( l->token_type == TOKEN_IDENTIFIER )
    {
        node->type = LITERAL_TYPE_IDENTIFIER;
        const size_t required_size = sizeof ( ast_literal_identifier_node ) + strlen ( p->current_token->value ) + 1;
        ast_literal_identifier_node* identifier = malloc ( required_size );
        strcpy ( identifier->name, p->current_token->value );
        identifier->real_value = malloc ( sizeof ( ast_literal_expression_node ) );
        node->identifier = identifier;
    }
    else if ( l->token_type == TOKEN_BOOLEAN )
    {
        node->type = LITERAL_TYPE_BOOLEAN;
        node->boolean = strcmp ( l->value, "TRUE" ) == 0 ? 1 : 0;
    }
    else
    {
        // unhandled
        exit ( 1 );
    }
    return node;
}

void handle_operation ( struct operator operator, stack * operands )
{
    ast_expression_node* first_operand = STACK_POP ( operands, ast_expression_node* );

    switch ( operator.operation )
    {
    case BINARY_OPERATION_PLUS:
    case BINARY_OPERATION_MINUS:
    case BINARY_OPERATION_MULTIPLICATION:
    case BINARY_OPERATION_DIVISION:
    case BINARY_GREATER_THAN:
    case BINARY_GREATER_THAN_OR_EQUAL:
    case BINARY_LESS_THAN:
    case BINARY_LESS_THAN_OR_EQUAL:
    case BINARY_EQUAL:
    case BINARY_NOT_EQUAL:
    case BINARY_OPERATION_AND:
    case BINARY_OPERATION_OR:
    case BINARY_LIKE:
    {
        ast_expression_node* other = STACK_POP ( operands, ast_expression_node* );
        ast_binary_expression_node* bin = malloc ( sizeof ( ast_binary_expression_node ) );

        bin->right_expression = first_operand;
        bin->left_expression = other;
        bin->operator= get_type_by_operation ( operator.operation );

        ast_expression_node* expression = malloc ( sizeof ( ast_expression_node ) );
        expression->type = EXPRESSION_BINARY;
        expression->binary = bin;
        STACK_PUSH ( operands, ast_expression_node*, expression );
    }
    break;
    case UNARY_OPERATION_MINUS:
    {
        ast_unary_expression_node* unary = malloc ( sizeof ( ast_unary_expression_node ) );
        unary->operator= OPERATOR_MINUS;
        unary->expression = first_operand;
        ast_expression_node* expression = malloc ( sizeof ( ast_expression_node ) );
        expression->type = EXPRESSION_UNARY;
        expression->unary = unary;
        STACK_PUSH ( operands, ast_expression_node*, expression );
    }
    break;
    case UNARY_NOT:
    {
        ast_unary_expression_node* unary = malloc ( sizeof ( ast_unary_expression_node ) );
        unary->operator= OPERATOR_NOT;
        unary->expression = first_operand;
        ast_expression_node* expression = malloc ( sizeof ( ast_expression_node ) );
        expression->type = EXPRESSION_UNARY;
        expression->unary = unary;
        STACK_PUSH ( operands, ast_expression_node*, expression );
    }
    break;
    default:
        exit ( 1 );
    }
}

void handle_precedence ( struct operator current, stack* operators, stack* operands )
{
    // https://en.wikipedia.org/wiki/Shunting_yard_algorithm
    while ( STACK_SIZE ( operators ) > 0 && STACK_PEEK ( operators, operator).operation != OPERATOR_LEFT_PARENTHESIS &&
            ( STACK_PEEK ( operators, operator).precedence > current.precedence || ( STACK_PEEK ( operators, operator).operation == current.operation && current.left_associative ) ) )
    {
        handle_operation ( STACK_POP ( operators, operator), operands );
    }
    STACK_PUSH ( operators, operator, current );
}

void handle_end_of_stream ( stack* operators, stack* operands )
{
    while ( STACK_SIZE ( operators ) > 0 )
    {
        handle_operation ( STACK_POP ( operators, operator), operands );
    }
}

operator build_operator ( const lexeme* const current, operator_type op_type )
{
    struct operator operator;
    operator.operation = 0;
    operator.precedence = 0;

    if ( op_type == OPERATOR_UNARY )
    {
        operator.left_associative = false;
        if ( current->token_type == TOKEN_MINUS )
        {
            operator.precedence = 2;
            operator.operation = UNARY_OPERATION_MINUS;
        }
        else if ( current->token_type == TOKEN_NOT )
        {
            operator.precedence = 5;
            operator.operation = UNARY_NOT;
        }
    }
    else if ( op_type == OPERATOR_BINARY )
    {
        operator.left_associative = true;
        if ( current->token_type == TOKEN_PLUS )
        {
            operator.operation = BINARY_OPERATION_PLUS;
            operator.precedence = 2;
        }
        else if ( current->token_type == TOKEN_DIVISION )
        {
            operator.operation = BINARY_OPERATION_DIVISION;
            operator.precedence = 3;
        }
        else if ( current->token_type == TOKEN_MINUS )
        {
            operator.operation = BINARY_OPERATION_MINUS;
            operator.precedence = 2;
        }
        else if ( current->token_type == TOKEN_MULTIPLY )
        {
            operator.operation = BINARY_OPERATION_MULTIPLICATION;
            operator.precedence = 3;
        }
    }
    return operator;
}

ast_expression_node* parse_function_expression ( parser* p )
{
    ast_function_expression* fn = malloc ( sizeof ( ast_function_expression ) );
    LOG_MESSAGE_DEBUG("Parsing function: %s ...\n", TOKEN_NAMES[p->current_token->token_type]);

    switch ( p->current_token->token_type )
    {
    case TOKEN_MAX:
    case TOKEN_MIN:
    case TOKEN_AVG:
    {

        if ( p->current_token->token_type == TOKEN_MIN )
        {
            fn->type = FUNCTION_TYPE_MIN;
        }
        else if ( p->current_token->token_type == TOKEN_MAX )
        {
            fn->type = FUNCTION_TYPE_MAX;
        }
        else if ( p->current_token->token_type == TOKEN_AVG )
        {
            fn->type = FUNCTION_TYPE_AVG;
        }
        else
        {
            exit ( 1 );
        }

        // min, max, avg both handle the arguments the same way

        queue* arguments = QUEUE_CONSTRUCT ( ast_expression_node*, 100 );

        parser_advance ( p );
        if ( p->current_token->token_type != TOKEN_LEFT_PARENTHESIS )
        {
            // left parenthesis is mandatory!
            exit ( 1 );
        }
        // drops the left parenthesis
        parser_advance ( p );

        bool expect_comma_or_end = false;

        while ( true )
        {
            if ( expect_comma_or_end )
            {
                if ( p->current_token->token_type == TOKEN_COMMA )
                {
                    // new operand
                    parser_advance ( p );
                }
                else if ( p->current_token->token_type == TOKEN_RIGHT_PARENTHESIS )
                {
                    parser_advance ( p );
                    break;
                }
                else
                {
                    // unexpected token
                    exit ( 1 );
                }
            }
            else
            {
                QUEUE_PUSH ( arguments, ast_expression_node*, parse_expression_2 ( p, TOKEN_COMMA, TOKEN_RIGHT_PARENTHESIS ) );
            }
            expect_comma_or_end = !expect_comma_or_end;
        }

        if ( QUEUE_SIZE ( arguments ) == 0 )
        {
            exit ( 1 );
            // no arguments found
        }

        ast_expression_list_node* list = malloc ( sizeof ( ast_expression_list_node ) );
        list->size = 0;
        list->expressions = ARRAY_CONSTRUCT ( ast_expression_node*, QUEUE_SIZE ( arguments ) );

        while ( QUEUE_SIZE ( arguments ) > 0 )
        {
            ARRAY_ADD ( list->expressions, list->size, QUEUE_POP ( arguments, ast_expression_node* ) );
            list->size++;
        }
        ast_expression_node* expression = malloc ( sizeof ( ast_expression_node ) );
        expression->type = EXPRESSION_LIST;
        expression->list = list;
        fn->expression = expression;

        QUEUE_DESTROY ( arguments );
    }
    break;
    case TOKEN_UPPER:
    case TOKEN_LOWER:
    {
        fn->type = p->current_token->token_type == TOKEN_UPPER ? FUNCTION_TYPE_UPPER : FUNCTION_TYPE_LOWER;
        parser_advance ( p );
        if ( p->current_token->token_type != TOKEN_LEFT_PARENTHESIS )
        {
            // left parenthesis is mandatory!
            exit ( 1 );
        }
        // drop the left parenthesis
        parser_advance ( p );
        fn->expression = parse_expression_1 ( p, TOKEN_RIGHT_PARENTHESIS );

        // drop the right parenthesis
        parser_advance ( p );
    }
    break;
    case TOKEN_DATE:
    {
        // drops the DATE
        parser_advance ( p );
        fn->type = FUNCTION_TYPE_DATE;
        if ( p->current_token->token_type != TOKEN_LEFT_PARENTHESIS )
        {
            // left parenthesis is mandatory!
            exit ( 1 );
        }
        // drop the left parenthesis
        parser_advance ( p );
        fn->expression = parse_expression_1 ( p, TOKEN_RIGHT_PARENTHESIS );

        // drop the right parenthesis
        parser_advance ( p );
    }
    break;
    default:
        fprintf ( stderr, "Unhandled function!\n" );
        exit ( 1 );
    }

    ast_expression_node* expression = malloc ( sizeof ( ast_expression_node ) );
    expression->type = EXPRESSION_FUNCTION;
    expression->function = fn;
    return expression;
}

bool is_unary_minus ( parser* p )
{
    if ( p->previous_token->token_class == TOKEN_CLASS_KEYWORD || p->previous_token->token_class == TOKEN_CLASS_LOGICAL_OPERATOR ||
         p->previous_token->token_class == TOKEN_CLASS_BINARY_ARITHMETIC_OPERATOR || p->previous_token->token_class == TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR ||
         p->previous_token->token_type == TOKEN_LEFT_PARENTHESIS || p->previous_token->token_type == TOKEN_MINUS )
    {
        return true;
    }
    return false;
}

operator handle_relational_operator ( const lexeme* const token )
{
    operator op;
    op.left_associative = true;
    op.precedence = 1;

    if ( token->token_type == TOKEN_LESS_THAN )
    {
        op.operation = BINARY_LESS_THAN;
    }
    else if ( token->token_type == TOKEN_LESS_THAN_OR_EQUAL )
    {
        op.operation = BINARY_LESS_THAN_OR_EQUAL;
    }
    else if ( token->token_type == TOKEN_GREATER_THAN )
    {
        op.operation = BINARY_GREATER_THAN;
    }
    else if ( token->token_type == TOKEN_GREATER_THAN_OR_EQUAL )
    {
        op.operation = BINARY_GREATER_THAN_OR_EQUAL;
    }
    else if ( token->token_type == TOKEN_EQUAL )
    {
        op.operation = BINARY_EQUAL;
    }
    else if ( token->token_type == TOKEN_NOT_EQUAL )
    {
        op.operation = BINARY_NOT_EQUAL;
    }
    else if ( token->token_type == TOKEN_LIKE )
    {
        op.operation = BINARY_LIKE;
    }
    return op;
}

int get_type_by_operation ( operation op )
{
    switch ( op )
    {
    case BINARY_OPERATION_PLUS:
        return OPERATOR_PLUS;
    case BINARY_OPERATION_MINUS:
        return OPERATOR_MINUS;
    case BINARY_OPERATION_MULTIPLICATION:
        return OPERATOR_MULTIPLY;
    case BINARY_OPERATION_DIVISION:
        return OPERATOR_DIVIDE;
    case BINARY_LESS_THAN:
        return OPERATOR_LESS;
    case BINARY_LESS_THAN_OR_EQUAL:
        return OPERATOR_LESS_THAN_OR_EQUAL;
    case BINARY_GREATER_THAN:
        return OPERATOR_GREATER;
    case BINARY_GREATER_THAN_OR_EQUAL:
        return OPERATOR_GREATER_THAN_OR_EQUAL;
    case BINARY_EQUAL:
        return OPERATOR_EQUAL;
    case BINARY_NOT_EQUAL:
        return OPERATOR_NOT_EQUAL;
    case BINARY_OPERATION_AND:
        return OPERATOR_AND;
    case BINARY_OPERATION_OR:
        return OPERATOR_OR;
    case UNARY_OPERATION_MINUS:
        return OPERATOR_MINUS;
    case BINARY_LIKE:
        return OPERATOR_LIKE;
    default:
        exit ( 1 );
    }
}

void parse_number_literal ( ast_literal_expression_node* node, const lexeme* const l )
{
    char* ptr = strchr ( l->value, '.' );
    bool is_real = false;
    if ( ptr != NULL )
    {
        ptr = ptr + 1;
        size_t length = strlen ( ptr );

        // if all the digits after the "dot" are 0, there is no need to parse it as a real
        for ( size_t i = 0; i < length; i++ )
        {
            if ( ptr[ i ] == '.' )
            {
                // invalid
                fprintf ( stderr, "SYNTAX error: The number %s contains two dots, what is invalid!\n", l->value );
                exit ( 1 );
            }

            if ( ptr[ i ] != '0' )
            {
                is_real = true;
                break;
            }
        }
    }

    if ( is_real )
    {
        node->type = LITERAL_TYPE_REAL;
        node->real = atof ( l->value );
    }
    else
    {
        node->type = LITERAL_TYPE_INTEGER;
        node->integer = atoi ( l->value );
    }
}

ast_expression_node* parse_expression_2 ( parser* p, token delimiter1, token delimiter2 )
{
    return parse_expression_3 ( p, delimiter1, delimiter2, delimiter2 );
}
ast_expression_node* parse_expression_1 ( parser* p, token delimiter1 )
{
    return parse_expression_3 ( p, delimiter1, delimiter1, delimiter1 );
}

ast_order_node* parse_order ( parser* p )
{
    if ( p->current_token == NULL || p->current_token->token_type != TOKEN_ORDER )
    {
        return NULL;
    }

    // drop the order keyword
    parser_advance ( p );

    if ( p->current_token->token_type != TOKEN_BY )
    {
        // its mandatory to have the BY keyword
        fprintf ( stderr, "SYNTAX error: Expected BY after ORDER" );
        exit ( 1 );
    }

    // drops the BY KEYWORD
    parser_advance ( p );

    // expects an integer
    if ( p->current_token->token_type != TOKEN_NUMBER )
    {
        fprintf ( stderr, "SYNTAX error: Expected a number after ORDER BY" );
        exit ( 1 );
    }

    ast_order_node* order = malloc ( sizeof ( ast_order_node ) );
    order->column_number = atoi ( p->current_token->value );

    // drops the number
    parser_advance ( p );

    if ( p->current_token->token_type == TOKEN_ASC )
    {
        order->mode = ORDER_ASC;
        parser_advance ( p );
    }
    else if ( p->current_token->token_type == TOKEN_DESC )
    {
        order->mode = ORDER_DESC;
        parser_advance ( p );
    }
    return order;
}

ast_limit_node* parse_limit ( parser* p )
{
    if ( p->current_token == NULL || p->current_token->token_type != TOKEN_LIMIT )
    {
        return NULL;
    }
    // consume the LIMIT keyword
    parser_advance ( p );

    // expects a number
    if ( p->current_token->token_type != TOKEN_NUMBER )
    {
        fprintf ( stderr, "SYNTAX error: Expected a number after limit clause ...\n" );
        exit ( 30 );
    }
    ast_limit_node* limit = calloc ( 1, sizeof ( ast_limit_node ) );
    limit->value = atoi ( p->current_token->value );
    return limit;
}