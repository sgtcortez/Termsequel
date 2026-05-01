#ifndef __TERMSEQUEL_LOGGER_H__
#define __TERMSEQUEL_LOGGER_H__

typedef enum 
{
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_ERROR = 2
} log_level;

#define LOG_MESSAGE_DEBUG(format, ...) log_message(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define LOG_MESSAGE_INFO(format, ...) log_message(LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define LOG_MESSAGE_ERROR(format, ...) log_message(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

void set_log_level(const log_level level);
void log_message(const log_level level, const char* const format, ...);



#endif