#ifndef TERMSEQUEL_EVALUATOR_H
#define TERMSEQUEL_EVALUATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "termsequel/backend/physical_plan.h"
#include "termsequel/frontend/node.h"

typedef struct evaluate_result evaluate_result;
typedef enum evaluate_result_type
{
    EVALUATE_INVALID = 0,
    EVALUATE_STRING,
    EVALUATE_INTEGER,
    EVALUATE_REAL,
    EVALUATE_BOOLEAN,
} evaluate_result_type;

struct evaluate_result
{
    evaluate_result_type type;
    union
    {
        bool boolean;
        int64_t integer;
        double real;
        char string[ 255 ];
    };
};

evaluate_result evaluate_node ( const ast_expression_node* const node, const physical_plan_scan_output* const scan );

#endif
