#ifndef __TERMSEQUEL_SYSTEM_H__
#define __TERMSEQUEL_SYSTEM_H__

#include <stdint.h>

/**
 Supported operation systems
*/

#define LINUX_OS 1
#define BSD_OS 2
#define APPLE_OS 3
#define WINDOWS_OS 4


#ifdef __linux__

#include <sys/stat.h>
#include <linux/stat.h>

typedef struct statx file_system_data;

#define OS_IN_USE LINUX_OS

#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)

#include <sys/stat.h>

typedef struct stat file_system_data;

#define OS_IN_USE BSD_OS

#elif defined(macintosh) || defined(Macintosh) || defined(__APPLE__) 

#include <sys/stat.h>

#define OS_IN_USE APPLE_OS

typedef struct stat file_system_data;

#else 

#define OS_IN_USE WINDOWS_OS

#error "Unsupported system!"

#endif

uint64_t system_file_data_size(const file_system_data* const file_system_data);
long long system_file_data_create_date(const file_system_data* const file_system_data);
long long system_file_data_modify_date(const file_system_data* const file_system_data);
uint16_t system_file_data_mode(const file_system_data* const file_system_data);
uint32_t system_file_data_uid(const file_system_data* const file_system_data);
uint32_t system_file_data_gid(const file_system_data* const file_system_data);
int32_t system_request_file_data(const char* const file_path, const int32_t flags, file_system_data* file_system_data);

#endif