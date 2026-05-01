#include "termsequel/backend/physical_plan.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

extern FILE* output_file;

static physical_plan_pipeline_output consume ( physical_plan* plan );

typedef struct iterator
{
    size_t produced;
    size_t max;
} iterator;

physical_plan* build_physical_plan_output ( const logical_plan_output* const output )
{
    iterator* it = calloc ( 1, sizeof ( iterator ) );
    it->max = output->limit;
    it->produced = 0;

    physical_plan* out = calloc ( 1, sizeof ( physical_plan ) );
    out->data = it;
    out->operation = PHYSICAL_PLAN_OPERATION_OUTPUT;
    out->consume = consume;
    return out;
}

physical_plan_pipeline_output consume ( physical_plan* plan )
{
    const size_t max_rows = ( (iterator*) plan->data )->max;
    size_t* produced = &( (iterator*) plan->data )->produced;
    while ( *produced <= max_rows )
    {
        physical_plan_pipeline_output output = plan->child->consume ( plan->child );
        if ( output.result == PHYSICAL_RESULT_EOF )
        {
            break;
        }
        else if ( output.result == PHYSICAL_RESULT_REMOVED )
        {
            continue;
        }

        physical_plan_project_output_row* row = output.data;
        for ( size_t column_index = 0; column_index < ARRAY_CAPACITY ( row->columns ); column_index++ )
        {
            physical_plan_projection column = *ARRAY_AT ( row->columns, column_index, physical_plan_projection );
            switch ( column.type )
            {
            case PHYSICAL_PLAN_PROJECTION_TYPE_BOOLEAN:
                fprintf ( output_file, "%-15s", ( column.boolean ? "TRUE" : "FALSE" ) );
                break;
            case PHYSICAL_PLAN_PROJECTION_TYPE_STRING:
                fprintf ( output_file, "%-15s", string_raw ( column.str ) );
                break;
            case PHYSICAL_PLAN_PROJECTION_TYPE_INTEGER:
                fprintf ( output_file, "%-15ld", column.integer );
                break;
            case PHYSICAL_PLAN_PROJECTION_TYPE_REAL:
                fprintf ( output_file, "%-15f", column.real );
                break;
            default:
                exit ( 1 );
            }
            fprintf ( output_file, " " );
        }
        ( *produced )++;

        fprintf ( output_file, "\n" );
        physical_plan_pipeline_output pipe;
        pipe.result = PHYSICAL_RESULT_OK;
        return pipe;
    }
    physical_plan_pipeline_output pipe;
    pipe.result = PHYSICAL_RESULT_EOF;
    return pipe;
}
