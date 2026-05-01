#include "termsequel/backend/system.h"

#if defined(OS_IN_USE) && (OS_IN_USE == APPLE_OS || OS_IN_USE == BSD_OS)

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

uint64_t system_file_data_size(const file_system_data* const file_system_data)
{
    return file_system_data->st_size;
}

long long system_file_data_create_date(const file_system_data* const file_system_data)
{
    return file_system_data->st_birthtimespec.tv_sec;
}

long long system_file_data_modify_date(const file_system_data* const file_system_data)
{
    return file_system_data->st_mtimespec.tv_sec;
}

uint16_t system_file_data_mode(const file_system_data* const file_system_data)
{
    return file_system_data->st_mode;
}

uint32_t system_file_data_uid(const file_system_data* const file_system_data)
{
    return file_system_data->st_uid;
}

uint32_t system_file_data_gid(const file_system_data* const file_system_data)
{
    return file_system_data->st_gid;
}

int32_t system_request_file_data(const char* const file_path, const int32_t flags, file_system_data* file_system_data)
{
    ((void)flags);
    if (file_system_data == NULL)
    {
        fprintf(stderr, "STATX buffer cannot be null!\n");
        return -1;
    }
    int result = stat ( file_path, file_system_data);
    if (result < 0)
    {
        fprintf(stderr, "Error with statx ...\n");
    }
    return result;
}


#endif