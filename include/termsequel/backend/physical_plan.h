#ifndef TERMSEQUEL_PHYSICAL_PLAN_H
#define TERMSEQUEL_PHYSICAL_PLAN_H

#include "cgeneric/array.h"
#include "logical_plan.h"
#include "termsequel/string/string.h"
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct physical_plan physical_plan;
typedef struct physical_plan_pipeline_output physical_plan_pipeline_output;
typedef struct physical_plan_scan_output physical_plan_scan_output;
typedef struct physical_plan_project_output_row physical_plan_project_output_row;
typedef struct physical_plan_projection physical_plan_projection;

typedef enum physical_plan_operation
{
    PHYSICAL_PLAN_OPERATION_INVALID = 0,
    PHYSICAL_PLAN_OPERATION_SCAN,
    PHYSICAL_PLAN_OPERATION_FILTER,
    PHYSICAL_PLAN_OPERATION_PROJECT,
    PHYSICAL_PLAN_OPERATION_SORT,
    PHYSICAL_PLAN_OPERATION_OUTPUT,
} physical_plan_operation;

typedef enum physical_plan_projection_type
{
    PHYSICAL_PLAN_PROJECTION_TYPE_INVALID = 0,
    PHYSICAL_PLAN_PROJECTION_TYPE_INTEGER,
    PHYSICAL_PLAN_PROJECTION_TYPE_REAL,
    PHYSICAL_PLAN_PROJECTION_TYPE_BOOLEAN,
    PHYSICAL_PLAN_PROJECTION_TYPE_STRING,
} physical_plan_projection_type;

extern char* PHYSICAL_PLAN_OPERATION_NAMES[];

struct physical_plan_projection
{
    physical_plan_projection_type type;
    union
    {
        int64_t integer;
        double real;
        bool boolean;
        string str;
    };
};

typedef enum scan_file_type
{
    SCAN_FILE_TYPE_UNKNOWN = 0,
    SCAN_FILE_TYPE_DIRECTORY,
    SCAN_FILE_TYPE_REGULAR_FILE,
} scan_file_type;

struct physical_plan_scan_output
{
    u_int8_t depth_level;
    u_int16_t owner_id;
    u_int16_t group_id;
    scan_file_type file_type;
    size_t size;
    char* name;
    char* owner_name;
    char* group_name;
    char* relative_path;
    char* owner_permissions;
    char* group_permissions;
    char* modify_date;
    char* create_date;
};

struct physical_plan_project_output_row
{
    array* columns;
};

typedef enum physical_operation_result
{
    // reached the end of the stream
    PHYSICAL_RESULT_EOF = 0,

    // file was removed due to filter not matching
    PHYSICAL_RESULT_REMOVED,

    // everything ok
    PHYSICAL_RESULT_OK,
} physical_operation_result;

struct physical_plan_pipeline_output
{
    physical_operation_result result;
    void* data;
};

struct physical_plan
{
    physical_plan_operation operation;
    void* data;
    physical_plan* child;
    physical_plan_pipeline_output ( *consume ) ( physical_plan* );
};

physical_plan* build_physical_plans ( const logical_plan_node* const plan );

#endif