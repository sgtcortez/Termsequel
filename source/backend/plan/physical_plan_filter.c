#include "termsequel/backend/physical_plan.h"
#include "termsequel/frontend/node.h"
#include "termsequel/log/logger.h"

#include "termsequel/backend/evaluator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static physical_plan_pipeline_output consume ( physical_plan* plan );
static void print_file_removed(const physical_plan_scan_output* scan, const ast_expression_node* const expression);

physical_plan* build_physical_plan_filter ( const logical_plan_filter* const filter )
{
    physical_plan* p = calloc ( 1, sizeof ( physical_plan ) );
    p->data = filter->expression;
    p->operation = PHYSICAL_PLAN_OPERATION_FILTER;
    p->consume = consume;
    return p;
}

physical_plan_pipeline_output consume ( physical_plan* plan )
{
    // filter will always consume the scan, 1:1 mapping
    physical_plan_pipeline_output out;

    physical_plan_pipeline_output child_out = plan->child->consume ( plan->child );
    if ( child_out.result != PHYSICAL_RESULT_OK )
    {
        return child_out;
    }

    // no need to free the child scaness
    physical_plan_scan_output* scan = child_out.data;

    ast_expression_node* expression = plan->data;

    if ( evaluate_node ( expression, scan ).boolean )
    {
        out.result = PHYSICAL_RESULT_OK;
        out.data = scan;
        return out;
    }
    print_file_removed(scan, expression);
    out.result = PHYSICAL_RESULT_REMOVED;
    return out;
}

static void print_file_removed(const physical_plan_scan_output* scan, const ast_expression_node* const expression)
{
    




    if (scan->name && scan->relative_path)
    {
        LOG_MESSAGE_DEBUG("File: %s at location \"%s\" was removed due to not matching the filter!\n", scan->name, scan->relative_path );
    }
    else if (scan->name)
    {
        LOG_MESSAGE_DEBUG("File: at location \"%s\" was removed due to not matching the filter!\n", scan->relative_path );
    }
    else if(scan->relative_path)
    {
        LOG_MESSAGE_DEBUG("File: %s was removed due to not matching the filter!\n", scan->name);
    }
}