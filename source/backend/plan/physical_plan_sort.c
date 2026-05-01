#include "cgeneric/array.h"
#include "termsequel/backend/evaluator.h"
#include "termsequel/backend/physical_plan.h"
#include "termsequel/frontend/node.h"
#include "termsequel/string/string.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

typedef struct sort_data
{
    size_t current_row;
    ast_order_node* order;
    array* sorted_rows;
} sort_data;

array* join_projection_rows ( physical_plan* projection_plan );

static physical_plan_pipeline_output consume ( physical_plan* plan );

static int compare ( physical_plan_project_output_row* left, physical_plan_project_output_row* right, int index, bool asc );

physical_plan* build_physical_plan_sort ( const logical_plan_sort* const sort )
{
    sort_data* st = calloc ( 1, sizeof ( sort_data ) );
    st->current_row = 0;
    st->order = sort->sort;
    st->sorted_rows = NULL;

    physical_plan* pp = calloc ( 1, sizeof ( physical_plan ) );
    pp->data = st;
    pp->consume = consume;
    pp->operation = PHYSICAL_PLAN_OPERATION_SORT;
    return pp;
}

static physical_plan_pipeline_output consume ( physical_plan* plan )
{
    sort_data* st = plan->data;
    if ( st->sorted_rows == NULL )
    {
        // join all the data
        st->sorted_rows = join_projection_rows ( plan->child );

        // start at index 1, because we ignore the column names
        for ( size_t i = 1; i < ARRAY_CAPACITY ( st->sorted_rows ) - 1; i++ )
        {
            physical_plan_project_output_row* left = *ARRAY_AT ( st->sorted_rows, i, physical_plan_project_output_row* );

            for ( size_t j = i + 1; j < ARRAY_CAPACITY ( st->sorted_rows ); j++ )
            {
                // TODO: Change it to not use bubble sort
                physical_plan_project_output_row* right = *ARRAY_AT ( st->sorted_rows, j, physical_plan_project_output_row* );
                if ( compare ( left, right, st->order->column_number - 1, st->order->mode == ORDER_ASC ) > 0 )
                {
                    physical_plan_project_output_row temp = *right;
                    *right = *left;
                    *left = temp;
                    continue;
                }
            }
        }
        st->current_row = 0;

        /**
         * When sorting, the first row that we need to return is the column metadata
         */
    }

    physical_plan_pipeline_output pipe;
    pipe.result = PHYSICAL_RESULT_EOF;

    if ( st->current_row >= ARRAY_CAPACITY ( st->sorted_rows ) )
    {
        return pipe;
    }
    pipe.result = PHYSICAL_RESULT_OK;
    pipe.data = *ARRAY_AT ( st->sorted_rows, st->current_row++, physical_plan_project_output_row* );
    return pipe;
}

int compare ( physical_plan_project_output_row* left, physical_plan_project_output_row* right, int index, bool asc )
{
    physical_plan_projection left_value = *ARRAY_AT ( left->columns, index, physical_plan_projection );
    physical_plan_projection right_value = *ARRAY_AT ( right->columns, index, physical_plan_projection );

    int order = asc ? 1 : -1;
    int diff = 0;

    if ( left_value.type == PHYSICAL_PLAN_PROJECTION_TYPE_INTEGER || left_value.type == PHYSICAL_PLAN_PROJECTION_TYPE_BOOLEAN )
    {
        diff = left_value.integer - right_value.integer;
    }
    else if ( left_value.type == PHYSICAL_PLAN_PROJECTION_TYPE_STRING )
    {
        diff = strcasecmp ( string_raw ( left_value.str ), string_raw ( right_value.str ) );
    }
    return order * diff;
}

array* join_projection_rows ( physical_plan* projection_plan )
{
    if ( projection_plan->operation != PHYSICAL_PLAN_OPERATION_PROJECT )
    {
        fprintf ( stderr, "To join projection rows, the plan must be the projection ...\n" );
        exit ( 40 );
    }

    array* rows = ARRAY_CONSTRUCT ( physical_plan_project_output_row*, 50 );
    size_t index = 0;
    for ( ;; )
    {
        physical_plan_pipeline_output output = projection_plan->consume ( projection_plan );
        if ( output.result == PHYSICAL_RESULT_EOF )
        {
            break;
        }
        else if ( output.result == PHYSICAL_RESULT_REMOVED )
        {
            continue;
        }

        physical_plan_project_output_row* row = output.data;
        if ( index >= ARRAY_CAPACITY ( rows ) )
        {
            ARRAY_RESIZE ( rows, ARRAY_CAPACITY ( rows ) + 10 );
        }
        ARRAY_ADD ( rows, index++, row );
    }
    ARRAY_RESIZE ( rows, index );
    return rows;
}