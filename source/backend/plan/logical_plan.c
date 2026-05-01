#include "termsequel/backend/logical_plan.h"
#include "cgeneric/array.h"
#include "termsequel/columns.h"
#include "termsequel/frontend/node.h"
#include "termsequel/log/logger.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

static logical_plan_node* build_from_select ( const ast_select_statement_node* const select );

static uint32_t get_identifiers ( const ast_expression_node* const expression );
static uint32_t get_identifiers_from_select ( const ast_select_statement_node* const select );

struct identifier_attribute
{
    scan_interested_attributes attribute;
    const char* name;
} identifier_attribute_ar[] = {
    { .attribute = SCAN_ATTRIBUTE_NAME, .name = SYSTEM_COLUMN_NAME_STRING },
    { .attribute = SCAN_ATTRIBUTE_SIZE, .name = SYSTEM_COLUMN_SIZE_STRING },
    { .attribute = SCAN_ATTRIBUTE_GROUP_ID, .name = SYSTEM_COLUMN_GROUP_ID_STRING },
    { .attribute = SCAN_ATTRIBUTE_GROUP_NAME, .name = SYSTEM_COLUMN_GROUP_NAME_STRING },
    { .attribute = SCAN_ATTRIBUTE_OWNER_ID, .name = SYSTEM_COLUMN_OWNER_ID_STRING },
    { .attribute = SCAN_ATTRIBUTE_OWNER_NAME, .name = SYSTEM_COLUMN_OWNER_NAME_STRING },
    {
        .attribute = SCAN_ATTRIBUTE_RELATIVE_PATH,
        .name = SYSTEM_COLUMN_RELATIVE_PATH_STRING,
    },
    { .attribute = SCAN_ATTRIBUTE_OWNER_PERMISSION, .name = SYSTEM_COLUMN_OWNER_PERMISSION_STRING },
    { .attribute = SCAN_ATTRIBUTE_GROUP_PERMISSION, .name = SYSTEM_COLUMN_GROUP_PERMISSION_STRING },
    { .attribute = SCAN_ATTRIBUTE_CREATE_DATE, .name = SYSTEM_COLUMN_CREATE_DATE_STRING },
    { .attribute = SCAN_ATTRIBUTE_MODIFY_DATE, .name = SYSTEM_COLUMN_MODIFY_DATE_STRING },

};

logical_plan_node* logical_plan_translate ( const ast_statement_node* const statement )
{
    if ( statement->type == STATEMENT_SELECT )
    {
        return build_from_select ( statement->select );
    }
    return NULL;
}

static logical_plan_node* build_from_select ( const ast_select_statement_node* const select )
{

    LOG_MESSAGE_DEBUG("Translating to SELECT plan ...\n");

    logical_plan_node* root = NULL;

    /** PROJECTION */
    logical_plan_node* projection = malloc ( sizeof ( logical_plan_node ) );
    projection->type = LOGICAL_PLAN_TYPE_PROJECT;
    projection->project.columns = select->columns;
    projection->project.columns_number = ARRAY_CAPACITY ( select->columns );

    /** SCAN */
    logical_plan_node* scan = malloc ( sizeof ( logical_plan_node ) );
    scan->type = LOGICAL_PLAN_TYPE_SCAN;
    scan->scan.identifiers = get_identifiers_from_select ( select );
    scan->child = NULL;
    memset ( scan->scan.value, 0, sizeof ( scan->scan ) / sizeof ( scan->scan.value[ 0 ] ) );
    strcpy ( scan->scan.value, select->source );

    logical_plan_node* filter = NULL;

    /** FILTER */
    if ( select->filter != NULL )
    {
        filter = malloc ( sizeof ( logical_plan_node ) );
        filter->type = LOGICAL_PLAN_TYPE_FILTER;
        filter->filter.expression = select->filter;

        projection->child = filter;
        filter->child = scan;
    }
    else
    {
        projection->child = scan;
    }

    /** SORT */
    if ( select->sort != NULL )
    {
        logical_plan_node* sort_plan = malloc ( sizeof ( logical_plan_node ) );
        sort_plan->type = LOGICAL_PLAN_TYPE_SORT;
        sort_plan->sort.sort = select->sort;

        sort_plan->child = projection;
        root = sort_plan;
    }
    else
    {
        root = projection;
    }

    logical_plan_node* output_node = calloc ( 1, sizeof ( logical_plan_node ) );
    output_node->child = root;
    output_node->type = LOGICAL_PLAN_TYPE_OUTPUT;
    if ( select->limit != NULL )
    {
        output_node->output.limit = select->limit->value;
    }
    else
    {
        output_node->output.limit = SIZE_MAX;
    }
    return output_node;
}

uint32_t get_identifiers ( const ast_expression_node* const expression )
{
    if ( expression->type == EXPRESSION_LITERAL )
    {
        if ( expression->literal->type != LITERAL_TYPE_IDENTIFIER )
        {
            return 0;
        }
        const ast_literal_identifier_node* const identifier = expression->literal->identifier;
        for ( size_t index = 0; index < sizeof ( identifier_attribute_ar ) / sizeof ( identifier_attribute_ar[ 0 ] ); index++ )
        {
            if ( strcasecmp ( identifier->name, identifier_attribute_ar[ index ].name ) == 0 )
            {
                return identifier_attribute_ar[ index ].attribute;
            }
        }
        LOG_MESSAGE_ERROR("Missing attribute: %s to match to correct scan!\n", identifier->name );
        exit ( 100 );
    }
    if ( expression->type == EXPRESSION_BINARY )
    {
        // handle left and right
        const uint32_t left = get_identifiers ( expression->binary->left_expression );
        const uint32_t rightt = get_identifiers ( expression->binary->right_expression );
        return left | rightt;
    }
    if ( expression->type == EXPRESSION_UNARY )
    {
        return get_identifiers ( expression->unary->expression );
    }
    if ( expression->type == EXPRESSION_FUNCTION )
    {
        return get_identifiers ( expression->function->expression );
    }
    if ( expression->type == EXPRESSION_LIST )
    {
        const ast_expression_list_node* const list = expression->list;
        uint32_t result = 0;
        for ( size_t index = 0; index < list->size; index++ )
        {
            const ast_expression_node* const argument = *ARRAY_AT ( list->expressions, index, ast_expression_node* );
            result |= get_identifiers ( argument );
        }
        return result;
    }
    return 0;
}

uint32_t get_identifiers_from_select ( const ast_select_statement_node* const select )
{
    uint32_t result = 0;
    // we need to traverse the columns list
    for ( size_t index = 0; index < ARRAY_CAPACITY ( select->columns ); index++ )
    {
        const ast_expression_node* expression = *ARRAY_AT ( select->columns, index, ast_expression_node* );
        result |= get_identifiers ( expression );
    }
    if ( select->filter != NULL )
    {
        result |= get_identifiers ( select->filter );
    }
    return result;
}