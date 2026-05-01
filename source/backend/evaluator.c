#include "termsequel/backend/evaluator.h"
#include "cgeneric/array.h"
#include "termsequel/backend/physical_plan.h"
#include "termsequel/columns.h"
#include "termsequel/frontend/node.h"
#include "termsequel/log/logger.h"

#define _XOPEN_SOURCE
#include <time.h>

#include <ctype.h>
#include <float.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static evaluate_result evaluate_literal ( const ast_literal_expression_node* const literal, const physical_plan_scan_output* const args );
static evaluate_result evaluate_binary ( const ast_binary_expression_node* const binary, const physical_plan_scan_output* const args );
static evaluate_result evaluate_function ( const ast_function_expression* const function, const physical_plan_scan_output* const args );
static evaluate_result evaluate_unary ( const ast_unary_expression_node* const unary, const physical_plan_scan_output* const args );

static bool evaluate_like_argument ( const char* value, const char* pattern );

static double get_as_double ( const evaluate_result ev );

evaluate_result evaluate_node ( const ast_expression_node* const node, const physical_plan_scan_output* const args )
{
    if ( node->type == EXPRESSION_LITERAL )
    {
        return evaluate_literal ( node->literal, args );
    }
    if ( node->type == EXPRESSION_BINARY )
    {
        return evaluate_binary ( node->binary, args );
    }
    if ( node->type == EXPRESSION_FUNCTION )
    {
        return evaluate_function ( node->function, args );
    }
    if ( node->type == EXPRESSION_UNARY )
    {
        return evaluate_unary ( node->unary, args );
    }
    exit ( 1 );
}

evaluate_result evaluate_literal ( const ast_literal_expression_node* const literal, const physical_plan_scan_output* const args )
{
    switch ( literal->type )
    {
    case LITERAL_TYPE_BOOLEAN:
    {
        evaluate_result r;
        r.type = EVALUATE_BOOLEAN;
        r.boolean = literal->boolean;
        return r;
    }
    break;
    case LITERAL_TYPE_INTEGER:
    {
        evaluate_result r;
        r.type = EVALUATE_INTEGER;
        r.integer = literal->integer;
        return r;
    }
    break;
    case LITERAL_TYPE_REAL:
    {
        evaluate_result r;
        r.type = EVALUATE_REAL;
        r.real = literal->real;
        return r;
    }
    break;
    case LITERAL_TYPE_STRING:
    {
        evaluate_result r;
        r.type = EVALUATE_STRING;
        memset ( r.string, 0, sizeof ( r.string ) / sizeof ( r.string[ 0 ] ) );
        strcpy ( r.string, literal->string );
        return r;
    }
    break;
    case LITERAL_TYPE_IDENTIFIER:
    {
        const ast_literal_identifier_node* const identifier = literal->identifier;
        evaluate_result r = { 0 };
        switch ( identifier->real_value->type )
        {
        case LITERAL_TYPE_INTEGER:
            r.type = EVALUATE_INTEGER;
            if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_SIZE_STRING ) == 0 )
            {
                r.integer = args->size;
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_DEPTH_LEVEL_STRING ) == 0 )
            {
                r.integer = args->depth_level;
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_OWNER_ID_STRING ) == 0 )
            {
                r.integer = args->owner_id;
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_GROUP_ID_STRING ) == 0 )
            {
                r.integer = args->group_id;
            }
            else
            {
                fprintf ( stderr, "Identifier: %s is not being handled!\n", identifier->name );
                exit ( 155 );
            }
            break;
        case LITERAL_TYPE_STRING:
            r.type = EVALUATE_STRING;
            if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_NAME_STRING ) == 0 )
            {
                strcpy ( r.string, args->name );
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_TYPE_STRING ) == 0 )
            {
                if ( args->file_type == SCAN_FILE_TYPE_DIRECTORY )
                {
                    strcpy ( r.string, "DIRECTORY" );
                }
                else if ( args->file_type == SCAN_FILE_TYPE_REGULAR_FILE )
                {
                    strcpy ( r.string, "REGULAR FILE" );
                }
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_RELATIVE_PATH_STRING ) == 0 )
            {
                strcpy ( r.string, args->relative_path );
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_OWNER_NAME_STRING ) == 0 )
            {
                strcpy ( r.string, args->owner_name );
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_GROUP_NAME_STRING ) == 0 )
            {
                strcpy ( r.string, args->group_name );
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_OWNER_PERMISSION_STRING ) == 0 )
            {
                strcpy ( r.string, args->owner_permissions );
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_GROUP_PERMISSION_STRING ) == 0 )
            {
                strcpy ( r.string, args->group_permissions );
            }
            else
            {
                fprintf ( stderr, "Identifier: %s is not being handled!\n", identifier->name );
                exit ( 155 );
            }
            break;
        case LITERAL_TYPE_DATE:
        {
            // the output of dates, will always be a string.
            // to compare it, we can use the lexigographical sort
            r.type = EVALUATE_STRING;
            if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_MODIFY_DATE_STRING ) == 0 )
            {
                strcpy ( r.string, args->modify_date );
            }
            else if ( strcasecmp ( identifier->name, SYSTEM_COLUMN_CREATE_DATE_STRING ) == 0 )
            {
                strcpy ( r.string, args->create_date );
            }
        }
        break;
        default:
            // other types are not supported
            exit ( 1 );
        }
        return r;
    }
    break;
    default:
        exit ( 1 );
    }
}

evaluate_result evaluate_binary ( const ast_binary_expression_node* const binary, const physical_plan_scan_output* const args )
{
    const evaluate_result left = evaluate_node ( binary->left_expression, args );
    const evaluate_result right = evaluate_node ( binary->right_expression, args );
    evaluate_result e = { 0 };

    switch ( binary->operator)
    {
    case OPERATOR_PLUS:
    {
        if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.type = EVALUATE_INTEGER;
            e.integer = left.integer + right.integer;
        }
        else
        {
            // if at least one is a real number, we threat both as a double
            e.type = EVALUATE_REAL;
            e.real = get_as_double ( left ) + get_as_double ( right );
        }
    }
    break;
    case OPERATOR_MINUS:
    {
        if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.type = EVALUATE_INTEGER;
            e.integer = left.integer - right.integer;
        }
        else
        {
            // if at least one is a real number, we threat both as a double
            e.type = EVALUATE_REAL;
            e.real = get_as_double ( left ) - get_as_double ( right );
        }
    }
    break;
    case OPERATOR_MULTIPLY:
    {
        if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.type = EVALUATE_INTEGER;
            e.integer = left.integer * right.integer;
        }
        else
        {
            // if at least one is a real number, we threat both as a double
            e.type = EVALUATE_REAL;
            e.real = get_as_double ( left ) * get_as_double ( right );
        }
    }
    break;
    case OPERATOR_DIVIDE:
    {
        if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.type = EVALUATE_INTEGER;
            e.integer = left.integer / right.integer;
        }
        else
        {
            e.type = EVALUATE_REAL;
            e.real = get_as_double ( left ) / get_as_double ( right );
        }
    }
    break;
    case OPERATOR_MOD:
    {
        if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.type = EVALUATE_INTEGER;
            e.integer = left.integer % right.integer;
        }
        else
        {
            e.type = EVALUATE_REAL;
            exit ( 1 );
            // e.real = fmod ( get_as_double ( left ), get_as_double ( right ) );
        }
    }
    break;
    case OPERATOR_AND:
    {
        e.type = EVALUATE_BOOLEAN;
        e.boolean = left.boolean && right.boolean;
    }
    break;
    case OPERATOR_OR:
    {
        e.type = EVALUATE_BOOLEAN;
        e.boolean = left.boolean || right.boolean;
    }
    break;
    case OPERATOR_GREATER:
    {
        e.type = EVALUATE_BOOLEAN;
        if ( left.type == EVALUATE_STRING && right.type == EVALUATE_STRING )
        {
            e.boolean = strcmp ( left.string, right.string ) > 0;
        }
        else if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.boolean = left.integer > right.integer;
        }
        else
        {
            // if at least one is a real number, we threat both as a double
            e.boolean = get_as_double ( left ) > get_as_double ( right );
        }
    }
    break;
    case OPERATOR_LESS:
    {
        e.type = EVALUATE_BOOLEAN;
        if ( left.type == EVALUATE_STRING && right.type == EVALUATE_STRING )
        {
            e.boolean = strcmp ( left.string, right.string ) < 0;
        }
        else if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.boolean = left.integer < right.integer;
        }
        else
        {
            e.boolean = get_as_double ( left ) < get_as_double ( right );
        }
    }
    break;
    case OPERATOR_LESS_THAN_OR_EQUAL:
    {
        e.type = EVALUATE_BOOLEAN;
        if ( left.type == EVALUATE_STRING && right.type == EVALUATE_STRING )
        {
            e.boolean = strcmp ( left.string, right.string ) <= 0;
        }
        else if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.boolean = left.integer <= right.integer;
        }
        else
        {
            e.boolean = get_as_double ( left ) <= get_as_double ( right );
        }
    }
    break;
    case OPERATOR_GREATER_THAN_OR_EQUAL:
    {
        e.type = EVALUATE_BOOLEAN;
        if ( left.type == EVALUATE_STRING && right.type == EVALUATE_STRING )
        {
            e.boolean = strcmp ( left.string, right.string ) >= 0;
        }
        else if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.boolean = left.integer >= right.integer;
        }
        else
        {
            e.boolean = get_as_double ( left ) >= get_as_double ( right );
        }
    }
    break;
    case OPERATOR_EQUAL:
    {
        e.type = EVALUATE_BOOLEAN;
        if ( left.type == EVALUATE_STRING && right.type == EVALUATE_STRING )
        {
            e.boolean = strcmp ( left.string, right.string ) == 0;
        }
        else if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.boolean = left.integer == right.integer;
        }
        else if ( left.type == EVALUATE_STRING && right.type == EVALUATE_STRING )
        {
            e.boolean = strcmp ( left.string, right.string ) == 0;
        }
        else
        {
            // FIXME: We should not compare floating points this way!
            // https://floating-point-gui.de/errors/comparison/
            e.boolean = get_as_double ( left ) == get_as_double ( right );
        }
    }
    break;
    case OPERATOR_NOT_EQUAL:
    {
        e.type = EVALUATE_BOOLEAN;
        if ( left.type == EVALUATE_STRING && right.type == EVALUATE_STRING )
        {
            e.boolean = strcmp ( left.string, right.string ) != 0;
        }
        else if ( left.type == EVALUATE_INTEGER && right.type == EVALUATE_INTEGER )
        {
            e.boolean = left.integer != right.integer;
        }
        else if ( left.type == EVALUATE_STRING && right.type == EVALUATE_STRING )
        {
            e.boolean = strcmp ( left.string, right.string ) != 0;
        }
        else
        {
            e.boolean = get_as_double ( left ) != get_as_double ( right );
        }
    }
    break;
    case OPERATOR_LIKE:
    {
        e.type = EVALUATE_BOOLEAN;
        // the like "argument" is always in the right side
        e.boolean = evaluate_like_argument ( left.string, right.string );
    }
    break;
    default:
        exit ( 1 );
    }
    return e;
}

evaluate_result evaluate_function ( const ast_function_expression* const function, const physical_plan_scan_output* const args )
{
    evaluate_result er = {};

    switch ( function->type )
    {
    case FUNCTION_TYPE_DATE:
    {
        const evaluate_result inner = evaluate_node ( function->expression, args );
        er.type = EVALUATE_STRING;

        struct tm time = { 0 };
        if ( strptime ( inner.string, DATE_PATTERN, &time ) == NULL )
        {
            // TODO
        }
        strcpy ( er.string, inner.string );
    }
    break;
    case FUNCTION_TYPE_UPPER:
    case FUNCTION_TYPE_LOWER:
    {

        const evaluate_result inner = evaluate_node ( function->expression, args );

        er.type = EVALUATE_STRING;
        int ( *fn ) ( int ) = function->type == FUNCTION_TYPE_UPPER ? toupper : tolower;
        for ( size_t index = 0; index < strlen ( inner.string ); index++ )
        {
            er.string[ index ] = fn ( inner.string[ index ] );
        }
    }
    break;
    case FUNCTION_TYPE_MAX:
    case FUNCTION_TYPE_MIN:
    case FUNCTION_TYPE_AVG:
    {
        ast_expression_list_node* arguments = function->expression->list;
        double current = 0;
        bool shall_threat_as_real = false;

        if ( function->type == FUNCTION_TYPE_MAX )
        {
            current = -DBL_MAX;
        }
        else if ( function->type == FUNCTION_TYPE_MIN )
        {
            current = DBL_MAX;
        }
        else if ( function->type == FUNCTION_TYPE_AVG )
        {
            current = 0;
        }

        for ( size_t index = 0; index < arguments->size; index++ )
        {
            ast_expression_node* argument = *ARRAY_AT ( arguments->expressions, index, ast_expression_node* );
            evaluate_result inner = evaluate_node ( argument, args );
            const double temp = inner.type == EVALUATE_REAL ? inner.real : inner.integer;

            if ( inner.type == EVALUATE_REAL )
            {
                shall_threat_as_real = true;
            }

            if ( function->type == FUNCTION_TYPE_MAX )
            {
                current = current > temp ? current : temp;
            }
            else if ( function->type == FUNCTION_TYPE_MIN )
            {
                current = current < temp ? current : temp;
            }
            else
            {
                current += temp;
            }
        }

        if ( function->type == FUNCTION_TYPE_AVG )
        {
            shall_threat_as_real = true;
            current = current / arguments->size;
        }

        if ( shall_threat_as_real )
        {
            er.type = EVALUATE_REAL;
            er.real = current;
        }
        else
        {
            er.type = EVALUATE_INTEGER;
            er.integer = current;
        }
    }
    break;
    default:
        exit ( 1 );
    }

    return er;
}

evaluate_result evaluate_unary ( const ast_unary_expression_node* const unary, const physical_plan_scan_output* const args )
{
    const evaluate_result value = evaluate_node ( unary->expression, args );

    evaluate_result er = {};

    if ( unary->operator== OPERATOR_MINUS )
    {
        er.type = value.type;
        if ( value.type == EVALUATE_INTEGER )
        {
            er.integer = value.integer * -1;
        }
        else if ( value.type == EVALUATE_REAL )
        {
            er.real = value.real * -1;
        }
    }
    else if ( unary->operator== OPERATOR_NOT )
    {
        er.type = EVALUATE_BOOLEAN;
        er.boolean = !value.boolean;
    }
    return er;
}

static double get_as_double ( const evaluate_result ev )
{
    if ( ev.type == EVALUATE_REAL )
    {
        return ev.real;
    }
    if ( ev.type == EVALUATE_INTEGER )
    {
        return ev.integer;
    }
    exit ( 1 );
}

bool evaluate_like_argument ( const char* value, const char* pattern )
{
    const size_t length = strlen ( pattern );
    const bool start_wildcard = pattern[ 0 ] == '%';
    const bool end_wildcard = length > 0 ? pattern[ length - 1 ] == '%' : false;

    if ( !start_wildcard && !end_wildcard )
    {
        // if there is no % at the beginning or at the ending, then, its simple an equal operator
        return strcmp ( value, pattern ) == 0;
    }

    char buffer[ 64 ] = { 0 };
    const size_t start_index = start_wildcard ? 1 : 0;
    const size_t end_index = end_wildcard ? length - 2 : length - 1;
    strncpy ( buffer, pattern + start_index, end_index );

    char* substring = strstr ( value, buffer );

    if ( substring == NULL )
    {
        return false;
    }

    if ( start_wildcard && end_wildcard )
    {
        // %pattern%
        return true;
    }

    char temp[ 100 ] = { 0 };

    if ( start_wildcard )
    {
        strcpy ( temp, substring );
    }
    if ( end_wildcard )
    {
        // pattern%
        size_t size = (intptr_t) substring - (intptr_t) value + strlen ( buffer );
        strncpy ( temp, value, size );
    }

    return strcmp ( temp, buffer ) == 0;
}