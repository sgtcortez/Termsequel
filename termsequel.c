#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cgeneric/array.h"
#include "termsequel/backend/logical_plan.h"
#include "termsequel/backend/physical_plan.h"
#include "termsequel/frontend/analyzer.h"
#include "termsequel/frontend/lexer.h"
#include "termsequel/frontend/node.h"
#include "termsequel/frontend/parser.h"
#include "termsequel/frontend/expression_debug.h"
#include "termsequel/log/logger.h"

uint8_t recursion_depth = 0;
char* sql = NULL;
FILE* debug_file = NULL;
FILE* output_file = NULL;

void run_command(const char* sql);
void run_interactive();


int main ( int argc, char** argv )
{

    for (int index = 0; index < argc; index++)
    {
        if (strncmp(argv[index], "--recursion-depth=", strlen("--recursion-depth=")) == 0)
        {
            recursion_depth = atoi(argv[index] + strlen("--recursion-depth="));
        }
        else if (strncmp(argv[index], "--sql=", 6) == 0)
        {
            sql = argv[index] + (sizeof(char) * strlen("--sql="));
        }
        else if (strncmp(argv[index], "--debug-file=", strlen("--debug-file=")) == 0) 
        {
            debug_file = fopen(argv[index] + (sizeof(char) * strlen("--debug-file=")), "wt");
        }
        else if (strncmp(argv[index], "--output-file=", strlen("--output-file=")) == 0) 
        {
            output_file = fopen(argv[index] + (sizeof(char) * strlen("--output-file=")), "wt");
        }
    }

    if (output_file == NULL)
    {
        LOG_MESSAGE_DEBUG("No log file provided, routing result to stdout!\n");
        output_file = stdout;
    }

    if (debug_file != NULL)
    {
        set_log_level(LOG_LEVEL_DEBUG);
    }

    if (sql)
    {
        run_command(sql);
    }
    else 
    {
        run_interactive();
    }
    
    // Close Resources
    if (debug_file != NULL)
    {
        fclose(debug_file);
    }

    if (output_file != NULL && output_file != stdout)
    {
        fclose(output_file);
    }
    return 0;
}

void run_command(const char* sql)
{   
    LOG_MESSAGE_DEBUG("Using single instruction mode ...\n");

    ast_statement_node* ast = parse_statement ( sql );

    print_statement(ast);

    analyzer_analyze ( ast );

    logical_plan_node* lplan = logical_plan_translate ( ast );
    physical_plan* pplan = build_physical_plans(lplan);

    while (pplan->consume(pplan).result != PHYSICAL_RESULT_EOF);
}

void run_interactive()
{
    LOG_MESSAGE_DEBUG("Using interactive mode ...\n");

    while (true)
    {
        char buffer[1024] = {0};
        fprintf(stdout, "Enter SQL command, or \"/exit\" to exit\n");
        fgets(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, stdin);

        // removes the trailing newline
        buffer[strlen(buffer) - 1] = 0;

        if (strcmp("/exit", buffer) == 0)
        {
            LOG_MESSAGE_DEBUG("Exitting the program ...\n");
            break;
        }
        run_command(buffer);
    }
}
