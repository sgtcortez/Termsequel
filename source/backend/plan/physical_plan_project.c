#include "cgeneric/array.h"
#include "termsequel/backend/physical_plan.h"

#include "termsequel/backend/evaluator.h"
#include "termsequel/frontend/node.h"
#include "termsequel/string/string.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct projection_data projection_data;

struct projection_data
{
    bool first_iterarion;
    array* columns;
};

static physical_plan_pipeline_output consume ( physical_plan* plan );

physical_plan* build_physical_plan_projection ( const logical_plan_project* const projection )
{
    projection_data* pd = calloc ( 1, sizeof ( projection_data ) );
    pd->first_iterarion = true;
    pd->columns = projection->columns;

    physical_plan* pp = calloc ( 1, sizeof ( physical_plan ) );
    pp->data = pd;
    pp->consume = consume;
    pp->operation = PHYSICAL_PLAN_OPERATION_PROJECT;
    return pp;
}

static physical_plan_pipeline_output consume ( physical_plan* plan )
{
    projection_data* pd = plan->data;

    if ( pd->first_iterarion )
    {
        // only in the first iteration, we must produce the "columns"
        pd->first_iterarion = false;

        physical_plan_project_output_row* output = malloc ( sizeof ( physical_plan_project_output_row ) );
        output->columns = ARRAY_CONSTRUCT ( physical_plan_projection, ARRAY_CAPACITY ( pd->columns ) );

        for ( size_t index = 0; index < ARRAY_CAPACITY ( pd->columns ); index++ )
        {
            const char* column_name = ( *ARRAY_AT ( pd->columns, index, ast_column_node ) ).name;

            physical_plan_projection ppp = { 0 };
            ppp.type = PHYSICAL_PLAN_PROJECTION_TYPE_STRING;
            ppp.str = string_construct ( column_name );

            ARRAY_ADD ( output->columns, index, ppp );
        }

        physical_plan_pipeline_output out;
        out.data = output;
        out.result = PHYSICAL_RESULT_OK;
        return out;
    }

    physical_plan_pipeline_output out;

    physical_plan_pipeline_output child_output = plan->child->consume ( plan->child );
    if ( child_output.result == PHYSICAL_RESULT_REMOVED )
    {
        out.result = PHYSICAL_RESULT_REMOVED;
        return out;
    }
    if ( child_output.result == PHYSICAL_RESULT_EOF )
    {
        out.result = PHYSICAL_RESULT_EOF;
        free ( pd );
        return out;
    }

    physical_plan_scan_output* scan = child_output.data;

    physical_plan_project_output_row* output = malloc ( sizeof ( physical_plan_project_output_row ) );
    output->columns = ARRAY_CONSTRUCT ( physical_plan_projection, ARRAY_CAPACITY ( pd->columns ) );

    for ( size_t index = 0; index < ARRAY_CAPACITY ( pd->columns ); index++ )
    {
        const ast_expression_node* expression = ( ARRAY_AT ( pd->columns, index, ast_column_node ) )->expression;
        evaluate_result ev = evaluate_node ( expression, scan );
        physical_plan_projection ppp = { 0 };

        switch ( ev.type )
        {
        case EVALUATE_INTEGER:
        {
            ppp.type = PHYSICAL_PLAN_PROJECTION_TYPE_INTEGER;
            ppp.integer = ev.integer;
        }
        break;
        case EVALUATE_REAL:
        {
            ppp.type = PHYSICAL_PLAN_PROJECTION_TYPE_REAL;
            ppp.real = ev.real;
        }
        break;
        case EVALUATE_STRING:
        {
            ppp.type = PHYSICAL_PLAN_PROJECTION_TYPE_STRING;
            ppp.str = string_construct ( ev.string );
        }
        break;
        case EVALUATE_BOOLEAN:
        {
            ppp.type = PHYSICAL_PLAN_PROJECTION_TYPE_BOOLEAN;
            ppp.boolean = ev.boolean;
        }
        break;
        default:
            exit ( 1 );
        }
        ARRAY_ADD ( output->columns, index, ppp );
    }
    out.data = output;
    out.result = PHYSICAL_RESULT_OK;
    return out;
}
