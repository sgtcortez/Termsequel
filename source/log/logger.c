#include "termsequel/log/logger.h"

#include <stdarg.h>
#include <stdio.h>

static log_level current = LOG_LEVEL_INFO;

extern FILE* debug_file;
extern FILE* output_file;

char* LOG_NAMES[] = {
    "DEBUG",
    "INFO",
    "ERROR"
};

void set_log_level(const log_level level)
{
    current = level;
}

void log_message(const log_level level, const char* const format, ...)
{
    if (current > level)
    {
        return;
    }
    FILE* output = output_file;
    if (level == LOG_LEVEL_DEBUG && debug_file)
    {
        output = debug_file;
    }
    va_list args;
    va_start(args, format);
    vfprintf(output, format, args);
    va_end(args);    
}
