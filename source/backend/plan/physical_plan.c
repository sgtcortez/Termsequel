#include "termsequel/backend/physical_plan.h"
#include "cgeneric/array.h"
#include "termsequel/backend/logical_plan.h"
#include "termsequel/frontend/node.h"
#include "termsequel/string/string.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

extern physical_plan* build_physical_plan_scan ( const logical_plan_scan* const scan );
extern physical_plan* build_physical_plan_filter ( const logical_plan_filter* const filter );
extern physical_plan* build_physical_plan_projection ( const logical_plan_project* const projection );
extern physical_plan* build_physical_plan_sort ( const logical_plan_sort* const sort );
extern physical_plan* build_physical_plan_output ( const logical_plan_output* const output );

physical_plan* build_physical_plans ( const logical_plan_node* const plan )
{
    logical_plan_node* node = plan;
    physical_plan* root = NULL;
    physical_plan* tail = NULL;

    while ( node != NULL )
    {
        physical_plan* p;
        switch ( node->type )
        {
        case LOGICAL_PLAN_TYPE_FILTER:
            p = build_physical_plan_filter ( &node->filter );
            break;
        case LOGICAL_PLAN_TYPE_SCAN:
            p = build_physical_plan_scan ( &node->scan );
            break;
        case LOGICAL_PLAN_TYPE_PROJECT:
            p = build_physical_plan_projection ( &node->project );
            break;
        case LOGICAL_PLAN_TYPE_SORT:
            p = build_physical_plan_sort ( &node->sort );
            break;
        case LOGICAL_PLAN_TYPE_OUTPUT:
            p = build_physical_plan_output ( &node->output );
            break;
        default:
            exit ( 1 );
        }
        assert ( p != NULL && "Plan cannot be NULL!" );
        assert ( p->consume != NULL && "Plan Consumer cannot be NULL!" );
        if ( root == NULL )
        {
            root = p;
            tail = p;
        }
        else
        {
            tail->child = p;
            tail = p;
        }
        node = node->child;
    }
    return root;
}
