#include "termsequel/frontend/expression_debug.h"
#include "termsequel/frontend/node.h"
#include <stddef.h>

#include "termsequel/log/logger.h"

void print_expression (const ast_expression_node* const expression );

void print_statement (const ast_statement_node* const statement )
{
    if ( statement->type == STATEMENT_SELECT )
    {
        const ast_select_statement_node* const select = statement->select;
        for ( size_t index = 0; index < ARRAY_CAPACITY ( select->columns ); index++ )
        {
            print_expression ( *ARRAY_AT ( select->columns, index, ast_expression_node* ) );
            LOG_MESSAGE_DEBUG("\n");
        }
        if ( select->filter != NULL )
        {
            print_expression ( select->filter );
        }
    }
    LOG_MESSAGE_DEBUG("\n");
}

void print_expression ( const ast_expression_node* const expression )
{
    switch ( expression->type )
    {
    case EXPRESSION_BINARY:
    {
        ast_binary_expression_node* binary_expression = expression->binary;
        LOG_MESSAGE_DEBUG("[BIN_EXPR_%s] ( ", OPERATOR_NAMES[binary_expression->operator]);
        print_expression ( binary_expression->left_expression );
        LOG_MESSAGE_DEBUG(" ");
        print_expression ( binary_expression->right_expression );
        LOG_MESSAGE_DEBUG(" ) ");
    }
    break;
    case EXPRESSION_LITERAL:
        LOG_MESSAGE_DEBUG("LIT_EXPR" );
        ast_literal_expression_node* literal_expression = expression->literal;
        switch ( literal_expression->type )
        {
        case LITERAL_TYPE_INTEGER:
            LOG_MESSAGE_DEBUG("_I( %ld", literal_expression->integer );
            break;
        case LITERAL_TYPE_STRING:
            LOG_MESSAGE_DEBUG("_S( %s", literal_expression->string );
            break;
        case LITERAL_TYPE_REAL:
            LOG_MESSAGE_DEBUG("_R( %f", literal_expression->real );
            break;
        case LITERAL_TYPE_BOOLEAN:
            LOG_MESSAGE_DEBUG("_B( %s", ( literal_expression->boolean ? "TRUE" : "FALSE" ) );
            break;
        case LITERAL_TYPE_IDENTIFIER:
            LOG_MESSAGE_DEBUG("_ID( %s", literal_expression->identifier->name );
            break;
        default:
            exit ( 1 );
        }
        LOG_MESSAGE_DEBUG(" ) " );
        break;
    case EXPRESSION_FUNCTION:
    {
        ast_function_expression* fn = expression->function;
        switch ( fn->type )
        {
        case FUNCTION_TYPE_MAX:
        case FUNCTION_TYPE_MIN:
        case FUNCTION_TYPE_AVG:
        {
            if ( fn->type == FUNCTION_TYPE_MAX )
            {
                LOG_MESSAGE_DEBUG("MAX( " );
            }
            else if ( fn->type == FUNCTION_TYPE_MIN )
            {
                LOG_MESSAGE_DEBUG("MIN( " );
            }
            else if ( fn->type == FUNCTION_TYPE_AVG )
            {
                LOG_MESSAGE_DEBUG("AVG( " );
            }
            ast_expression_list_node* list = fn->expression->list;
            size_t index = 0;
            for ( ; index < ARRAY_CAPACITY ( list->expressions ) - 1; index++ )
            {
                print_expression ( *ARRAY_AT ( list->expressions, index, ast_expression_node* ) );
                LOG_MESSAGE_DEBUG(", " );
            }
            print_expression ( *ARRAY_AT ( list->expressions, index, ast_expression_node* ) );
            LOG_MESSAGE_DEBUG(" ) " );
        }
        break;
        case FUNCTION_TYPE_LOWER:
        case FUNCTION_TYPE_UPPER:
        case FUNCTION_TYPE_DATE:
        {
            if ( fn->type == FUNCTION_TYPE_LOWER )
            {
                LOG_MESSAGE_DEBUG("LOWER( " );
            }
            else if ( fn->type == FUNCTION_TYPE_UPPER )
            {
                LOG_MESSAGE_DEBUG("UPPER( " );
            }
            else if (fn->type == FUNCTION_TYPE_DATE)
            {
                LOG_MESSAGE_DEBUG("DATE( ");
            }
            print_expression ( fn->expression );
            LOG_MESSAGE_DEBUG(" ) " );
        }
        break;
        default:
            break;
        }
    }
    break;
    case EXPRESSION_UNARY:
    {
        ast_unary_expression_node* unary = expression->unary;
        if ( unary->operator== OPERATOR_MINUS )
        {
            LOG_MESSAGE_DEBUG("UNARY_MINUS( " );
            print_expression ( unary->expression );
            LOG_MESSAGE_DEBUG(" ) " );
        }
        else if ( unary->operator== OPERATOR_NOT )
        {
            LOG_MESSAGE_DEBUG("UNARY_NOT( " );
            print_expression ( unary->expression );
            LOG_MESSAGE_DEBUG(" ) " );
        }
    }
    break;
    default:
        LOG_MESSAGE_ERROR("Invalid expression type ...\n");
        break;
    }
}
