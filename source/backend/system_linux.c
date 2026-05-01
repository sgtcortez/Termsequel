#include "termsequel/backend/system.h"

#if defined(OS_IN_USE) && (OS_IN_USE == LINUX_OS)

#include <sys/stat.h>
#include <linux/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <unistd.h>

#define STATX( relative_path, flags, stat_buffer ) syscall ( __NR_statx, AT_FDCWD, relative_path, AT_SYMLINK_NOFOLLOW, flags, stat_buffer )

uint64_t system_file_data_size(const file_system_data* const file_system_data)
{
    return file_system_data->stx_size;
}

long long system_file_data_create_date(const file_system_data* const file_system_data)
{
    return file_system_data->stx_btime.tv_sec;
}

long long system_file_data_modify_date(const file_system_data* const file_system_data)
{
    return file_system_data->stx_mtime.tv_sec;
}

uint16_t system_file_data_mode(const file_system_data* const file_system_data)
{
    return file_system_data->stx_mode;
}

uint32_t system_file_data_uid(const file_system_data* const file_system_data)
{
    return file_system_data->stx_uid;
}

uint32_t system_file_data_gid(const file_system_data* const file_system_data)
{
    return file_system_data->stx_gid;
}

int32_t system_request_file_data(const char* const file_path, const int32_t flags, file_system_data* file_system_data)
{
    if (file_system_data == NULL)
    {
        fprintf(stderr, "STATX buffer cannot be null!\n");
        return -1;
    }
    int result = STATX ( file_path, flags, file_system_data);
    if (result < 0)
    {
        fprintf(stderr, "Error with statx ...\n");
    }
    return result;
}

#endif
