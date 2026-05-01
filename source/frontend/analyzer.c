#include "termsequel/frontend/analyzer.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <time.h>

#include "termsequel/columns.h"

#include "cgeneric/array.h"
#include "termsequel/frontend/node.h"

typedef struct column_info
{
    const char* name;
    ast_node_kind type;
    ast_literal_type literal;
} column_info;
static column_info supported_columns[] = {
    {
        .name = SYSTEM_COLUMN_NAME_STRING,
        .type = KIND_STRING,
        .literal = LITERAL_TYPE_STRING,
    },
    {
        .name = SYSTEM_COLUMN_DEPTH_LEVEL_STRING,
        .type = KIND_INTEGER,
        .literal = LITERAL_TYPE_INTEGER,
    },
    {
        .name = SYSTEM_COLUMN_SIZE_STRING,
        .type = KIND_INTEGER,
        .literal = LITERAL_TYPE_INTEGER,
    },
    { .name = SYSTEM_COLUMN_TYPE_STRING, .type = KIND_STRING, .literal = LITERAL_TYPE_STRING },
    {
        .name = SYSTEM_COLUMN_RELATIVE_PATH_STRING,
        .type = KIND_STRING,
        .literal = LITERAL_TYPE_STRING,
    },
    { .name = SYSTEM_COLUMN_OWNER_ID_STRING, .type = KIND_INTEGER, .literal = LITERAL_TYPE_INTEGER },
    { .name = SYSTEM_COLUMN_OWNER_NAME_STRING, .type = KIND_STRING, .literal = LITERAL_TYPE_STRING },
    { .name = SYSTEM_COLUMN_GROUP_ID_STRING, .type = KIND_INTEGER, .literal = LITERAL_TYPE_INTEGER },
    { .name = SYSTEM_COLUMN_GROUP_NAME_STRING, .type = KIND_STRING, .literal = LITERAL_TYPE_STRING },
    { .name = SYSTEM_COLUMN_OWNER_PERMISSION_STRING, .type = KIND_STRING, .literal = LITERAL_TYPE_STRING },
    { .name = SYSTEM_COLUMN_GROUP_PERMISSION_STRING, .type = KIND_STRING, .literal = LITERAL_TYPE_STRING },
    { .name = SYSTEM_COLUMN_CREATE_DATE_STRING, .type = KIND_DATE, .literal = LITERAL_TYPE_DATE },
    { .name = SYSTEM_COLUMN_MODIFY_DATE_STRING, .type = KIND_DATE, .literal = LITERAL_TYPE_DATE },

};
static const size_t supported_columns_count = sizeof ( supported_columns ) / sizeof ( supported_columns[ 0 ] );
static column_info* find_column ( const char* name );
static void resolve_node_kinds ( ast_expression_node* node );
static void resolve_function_node_kinds ( ast_expression_node* node );
static void resolve_binary_node_kinds ( ast_expression_node* node );

bool analyzer_analyze ( ast_statement_node* statement )
{

    if ( statement->type == STATEMENT_SELECT )
    {
        ast_select_statement_node* select = statement->select;
        for ( size_t i = 0; i < ARRAY_CAPACITY ( select->columns ); i++ )
        {
            resolve_node_kinds ( *ARRAY_AT ( select->columns, i, ast_expression_node* ) );
        }
        if ( select->filter != NULL )
        {
            if ( select->filter->type != EXPRESSION_BINARY && select->filter->type != EXPRESSION_UNARY )
            {
                fprintf ( stderr, "Filter must be a unary/binary expression!" );
                exit ( 1 );
            }
            resolve_node_kinds ( select->filter );
            if ( select->filter->kind != KIND_BOOLEAN )
            {
                fprintf ( stderr, "Filter must return a boolean value!" );
                exit ( 1 );
            }
        }
        if ( select->sort != NULL )
        {
            ast_order_node* order = select->sort;
            while ( order != NULL )
            {
                if ( order->column_number > ARRAY_CAPACITY ( select->columns ) )
                {
                    fprintf ( stderr, "SEMANTIC: Sort by column: %d its invalid because colum does not exist in projection ...", order->column_number );
                    exit ( 1 );
                }
                order = order->next;
            }
        }
    }
    return true;
}

static void resolve_node_kinds ( ast_expression_node* node )
{
    if ( node->type == EXPRESSION_LITERAL )
    {
        ast_literal_expression_node* literal = node->literal;
        switch ( literal->type )
        {
        case LITERAL_TYPE_STRING:
            node->kind = KIND_STRING;
            break;
        case LITERAL_TYPE_REAL:
            node->kind = KIND_REAL;
            break;
        case LITERAL_TYPE_INTEGER:
            node->kind = KIND_INTEGER;
            break;
        case LITERAL_TYPE_DATE:
            node->kind = KIND_DATE;
            break;
        case LITERAL_TYPE_IDENTIFIER:
        {
            column_info* col = find_column ( literal->identifier->name );
            if ( col == NULL )
            {
                fprintf ( stderr, "Semantics Error: Unknown column '%s'\n", literal->identifier->name );
                exit ( 1 );
            }
            node->kind = col->type;
            node->literal->identifier->real_value->type = col->literal;
        }
        break;
        case LITERAL_TYPE_BOOLEAN:
            node->kind = KIND_BOOLEAN;
            break;
        default:
            exit ( 1 );
        }
    }
    else if ( node->type == EXPRESSION_BINARY )
    {
        resolve_binary_node_kinds ( node );
    }
    else if ( node->type == EXPRESSION_UNARY )
    {
        resolve_node_kinds ( node->unary->expression );
        node->kind = node->unary->expression->kind;

        if ( node->unary->operator== OPERATOR_MINUS && !( node->kind & KIND_ANY_NUMBER ) )
        {
            fprintf ( stderr, "Semantics Error: Unary MINUS operator is only allowed with numbers" );
            exit ( 1 );
        }

        if ( node->unary->operator== OPERATOR_NOT && node->kind != KIND_BOOLEAN )
        {
            fprintf ( stderr, "Semantics Error: Unary NOT operator is only allowed with booleans" );
            exit ( 1 );
        }
    }
    else if ( node->type == EXPRESSION_FUNCTION )
    {
        resolve_function_node_kinds ( node );
    }
}

static column_info* find_column ( const char* name )
{
    for ( size_t i = 0; i < supported_columns_count; i++ )
    {
        if ( strcasecmp ( supported_columns[ i ].name, name ) == 0 )
        {
            return &supported_columns[ i ];
        }
    }
    return NULL;
}

static void resolve_function_node_kinds ( ast_expression_node* node )
{
    ast_function_expression* fn = node->function;
    switch ( fn->type )
    {
    case FUNCTION_TYPE_UPPER:
    case FUNCTION_TYPE_LOWER:
    {
        // expects just one argument, and must be a string
        resolve_node_kinds ( fn->expression );
        node->kind = fn->expression->kind;
        if ( node->kind != KIND_STRING )
        {
            fprintf ( stderr, "Semantics Error: UPPER/LOWER expects a string" );
            exit ( 1 );
        }
    }
    break;
    case FUNCTION_TYPE_MAX:
    case FUNCTION_TYPE_MIN:
    case FUNCTION_TYPE_AVG:
    {
        // we support a mix of real and integer values in max/min/avg functions
        for ( size_t i = 0; i < fn->expression->list->size; i++ )
        {
            ast_expression_node* expression = *ARRAY_AT ( fn->expression->list->expressions, i, ast_expression_node* );
            resolve_node_kinds ( expression );
            if ( expression->kind == KIND_STRING )
            {
                fprintf ( stderr, "Semantics Error: AVG/MAX/MIN expects only numbers" );
                exit ( 1 );
            }
        }
        node->kind = KIND_ANY_NUMBER;
    }
    break;
    case FUNCTION_TYPE_DATE:
    {
        resolve_node_kinds ( fn->expression );
        if ( fn->expression->kind != KIND_STRING )
        {
            fprintf ( stderr, "Semantics Error: DATE function expects a string argument.\n" );
            exit ( 13 );
        }
        node->kind = KIND_DATE;
    }
    break;
    default:
        exit ( 1 );
    }
}

static void resolve_binary_node_kinds ( ast_expression_node* node )
{
    ast_binary_expression_node* binary = node->binary;
    ast_expression_node* left = binary->left_expression;
    ast_expression_node* right = binary->right_expression;

    resolve_node_kinds ( left );
    resolve_node_kinds ( right );

    switch ( binary->operator)
    {
    case OPERATOR_PLUS:
    case OPERATOR_MINUS:
    case OPERATOR_MULTIPLY:
    case OPERATOR_DIVIDE:
    case OPERATOR_MOD:
    {
        // only numbers are allowed
        if ( !( left->kind & KIND_ANY_NUMBER ) || !( right->kind & KIND_ANY_NUMBER ) )
        {
            fprintf ( stderr, "Semantics Error: Plus/Minus/Multiplication/Divide/Module are only allowed with numbers!" );
            exit ( 1 );
        }
        node->kind = KIND_ANY_NUMBER;
    }
    break;
    case OPERATOR_AND:
    case OPERATOR_OR:
    {
        // only booleans are allowed
        if ( left->kind != right->kind || left->kind != KIND_BOOLEAN )
        {
            fprintf ( stderr, "Semantics Error: AND/OR are only allowed with booleans!" );
            exit ( 1 );
        }
        node->kind = KIND_BOOLEAN;
    }
    break;
    case OPERATOR_EQUAL:
    case OPERATOR_NOT_EQUAL:
    case OPERATOR_LESS:
    case OPERATOR_LESS_THAN_OR_EQUAL:
    case OPERATOR_GREATER:
    case OPERATOR_GREATER_THAN_OR_EQUAL:
    {
        if ( ( left->kind != right->kind ) && !( left->kind & right->kind ) )
        {
            fprintf ( stderr, "Semantics Error: \"=/!=/</<=/>/>=\" are only supported with same types" );
            exit ( 1 );
        }
        node->kind = KIND_BOOLEAN;
    }
    break;
    case OPERATOR_LIKE:
    {
        if ( left->kind != KIND_STRING || right->kind != KIND_STRING )
        {
            fprintf ( stderr, "Semantics Error: LIKE operator is only allowed with strigs!" );
            exit ( 1 );
        }
        node->kind = KIND_BOOLEAN;
    }
    break;
    default:
        fprintf ( stderr, "Semantics error unknow operator ...\n" );
        exit ( 1 );
    }
}