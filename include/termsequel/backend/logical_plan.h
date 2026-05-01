#ifndef TERMSEQUEL_LOGICAL_PLAN_H
#define TERMSEQUEL_LOGICAL_PLAN_H

#include "cgeneric/array.h"
#include "termsequel/frontend/node.h"

#include <stddef.h>
#include <stdint.h>

typedef enum logical_plan_type
{
    LOGICAL_PLAN_TYPE_INVALID = 0,
    LOGICAL_PLAN_TYPE_SCAN,
    LOGICAL_PLAN_TYPE_FILTER,
    LOGICAL_PLAN_TYPE_PROJECT,
    LOGICAL_PLAN_TYPE_SORT,
    LOGICAL_PLAN_TYPE_AGGREGATE,
    LOGICAL_PLAN_TYPE_OUTPUT,
} logical_plan_type;

typedef enum scan_interested_attributes
{
    SCAN_ATTRIBUTE_INVALID = 0,
    SCAN_ATTRIBUTE_SIZE = 1 << 1,
    SCAN_ATTRIBUTE_OWNER_ID = 1 << 2,
    SCAN_ATTRIBUTE_OWNER_NAME = 1 << 3,
    SCAN_ATTRIBUTE_GROUP_ID = 1 << 4,
    SCAN_ATTRIBUTE_GROUP_NAME = 1 << 5,
    SCAN_ATTRIBUTE_CREATE_DATE = 1 << 6,
    SCAN_ATTRIBUTE_MODIFY_DATE = 1 << 7,
    SCAN_ATTRIBUTE_NAME = 1 << 8,
    SCAN_ATTRIBUTE_RELATIVE_PATH = 1 << 9,
    SCAN_ATTRIBUTE_FILE_TYPE = 1 << 10,
    SCAN_ATTRIBUTE_OWNER_PERMISSION = 1 << 11,
    SCAN_ATTRIBUTE_GROUP_PERMISSION = 1 << 12,
} scan_interested_attributes;

typedef struct logical_plan_node logical_plan_node;
typedef struct logical_plan_scan logical_plan_scan;
typedef struct logical_plan_filter logical_plan_filter;
typedef struct logical_plan_project logical_plan_project;
typedef struct logical_plan_sort logical_plan_sort;
typedef struct logical_plan_output logical_plan_output;

struct logical_plan_scan
{
    uint32_t identifiers;
    char value[ 40 ];
};

struct logical_plan_filter
{
    ast_expression_node* expression;
};

struct logical_plan_project
{
    size_t columns_number;
    array* columns;
};

struct logical_plan_sort
{
    ast_order_node* sort;
};

struct logical_plan_output
{
    size_t limit;
};

struct logical_plan_node
{
    logical_plan_type type;
    logical_plan_node* child;
    union
    {
        logical_plan_scan scan;
        logical_plan_filter filter;
        logical_plan_project project;
        logical_plan_sort sort;
        logical_plan_output output;
    };
};

logical_plan_node* logical_plan_translate ( const ast_statement_node* const statement );

#endif