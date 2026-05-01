#include "termsequel/backend/physical_plan.h"
#include "termsequel/columns.h"
#include "cgeneric/array.h"
#include "cgeneric/stack.h"
#include "termsequel/backend/logical_plan.h"
#include "termsequel/backend/system.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <time.h>

extern uint8_t recursion_depth;

static void delete_directories ( void* data, void* buffer );

typedef struct directory_information directory_information;

struct scan_data
{
    bool is_directory;
    int32_t statx_flags;
    uint32_t identifiers;
    stack* directories;
};

struct directory_information
{
    DIR* pointer;
    char name[ 255 ];
};

static struct dirent* get_next_entry ( struct scan_data* sd );

static physical_plan_pipeline_output consume ( physical_plan* plan );

physical_plan* build_physical_plan_scan ( const logical_plan_scan* const scan )
{
    physical_plan* pp = calloc ( 1, sizeof ( physical_plan ) );
    pp->consume = consume;
    pp->operation = PHYSICAL_PLAN_OPERATION_SCAN;

    struct scan_data* sd = calloc ( 1, sizeof ( struct scan_data ) );
    sd->statx_flags = 0;
    sd->identifiers = scan->identifiers;

#if defined(OS_IN_USE) && (OS_IN_USE == LINUX_OS)
    // this one will always be required, since we need information of the filetype.
    sd->statx_flags = STATX_TYPE;

    if ( scan->identifiers & SCAN_ATTRIBUTE_SIZE )
    {
        sd->statx_flags |= STATX_SIZE;
    }
    if ( scan->identifiers & SCAN_ATTRIBUTE_GROUP_ID || scan->identifiers & SCAN_ATTRIBUTE_GROUP_NAME )
    {
        sd->statx_flags |= STATX_GID;
    }
    if ( scan->identifiers & SCAN_ATTRIBUTE_OWNER_ID || scan->identifiers & SCAN_ATTRIBUTE_OWNER_NAME )
    {
        sd->statx_flags |= STATX_UID;
    }
    if ( scan->identifiers & SCAN_ATTRIBUTE_CREATE_DATE )
    {
        sd->statx_flags |= STATX_BTIME;
    }
    if ( scan->identifiers & SCAN_ATTRIBUTE_MODIFY_DATE )
    {
        sd->statx_flags |= STATX_MTIME;
    }
    if ( scan->identifiers & ( SCAN_ATTRIBUTE_OWNER_PERMISSION | SCAN_ATTRIBUTE_OWNER_PERMISSION ) )
    {
        sd->statx_flags |= STATX_MODE;
    }
#endif

    struct stat st;

    if ( stat ( scan->value, &st ) < 0 )
    {
        // an error while opening the file
        fprintf ( stderr, "An error while trying to obtain %s information. Error: %s\n", scan->value, strerror ( errno ) );
        exit ( 6 );
    }

    if ( S_ISDIR ( st.st_mode ) )
    {
        // its a directory
        sd->is_directory = true;

        sd->directories = STACK_CONSTRUCT ( directory_information, recursion_depth + 1 );

        directory_information di = { 0 };
        strcpy ( di.name, scan->value );
        di.pointer = opendir ( di.name );

        STACK_PUSH ( sd->directories, directory_information, di );
    }
    else if ( S_ISREG ( st.st_mode ) )
    {
        fprintf ( stderr, "File: %s is a regular file ... What is not supported yet!\n", scan->value );
        exit ( 1 );
        // sd->is_directory = false;
        // sd->file = fopen(sd->name, "rt");
    }
    else
    {
        fprintf ( stderr, "File: %s has an unsupported file type ...!\n", scan->value );
        exit ( 1 );
    }

    pp->data = sd;
    return pp;
}

physical_plan_pipeline_output consume ( physical_plan* plan )
{
    physical_plan_pipeline_output out;
    struct scan_data* sd = plan->data;
    if ( sd->is_directory )
    {
        // read from directory

        struct dirent* entry = get_next_entry ( sd );
        if ( entry == NULL )
        {
            // reached the end
            out.result = PHYSICAL_RESULT_EOF;
            STACK_FOREACH ( sd->directories, directory_information, delete_directories, NULL );
            STACK_DESTROY ( sd->directories );
            return out;
        }
        if ( entry->d_name[ 0 ] == '.' && ( entry->d_name[ 1 ] == 0 || ( entry->d_name[ 1 ] == '.' && entry->d_name[ 2 ] == 0 ) ) )
        {
            // we ignore the pseudo files
            out.result = PHYSICAL_RESULT_REMOVED;
            return out;
        }

        char filepath[ 2048 ] = { 0 };
        directory_information di = STACK_PEEK ( sd->directories, directory_information );
        snprintf ( filepath, sizeof ( filepath ), "%s/%s", di.name, entry->d_name );

        file_system_data fsd = {0};

        int result = system_request_file_data ( filepath, sd->statx_flags, &fsd );

        if ( result == -1 )
        {
            // FIXME
            fprintf ( stderr, "An error with stat of file: %s. Error: %s\n", filepath, strerror ( errno ) );
            exit ( 10 );
        }

        physical_plan_scan_output* scan = calloc ( 1, sizeof ( physical_plan_scan_output ) );

        scan->depth_level = STACK_SIZE ( sd->directories ) - 1;

        if ( sd->identifiers & SCAN_ATTRIBUTE_SIZE )
        {
            scan->size = system_file_data_size(&fsd);
        }
        if ( sd->identifiers & SCAN_ATTRIBUTE_NAME )
        {
            scan->name = strdup ( entry->d_name );
        }
        if ( sd->identifiers & SCAN_ATTRIBUTE_RELATIVE_PATH )
        {
            scan->relative_path = strdup ( filepath );
        }
        if ( sd->identifiers & SCAN_ATTRIBUTE_FILE_TYPE )
        {
            if ( S_ISDIR ( system_file_data_mode(&fsd) ) )
            {
                scan->file_type = SCAN_FILE_TYPE_DIRECTORY;
            }
            else if ( S_ISREG ( system_file_data_mode(&fsd) ) )
            {
                scan->file_type = SCAN_FILE_TYPE_REGULAR_FILE;
            }
        }
        if ( sd->identifiers & SCAN_ATTRIBUTE_OWNER_ID )
        {
            scan->owner_id = system_file_data_uid(&fsd);
        }

        if ( sd->identifiers & SCAN_ATTRIBUTE_GROUP_ID )
        {
            scan->group_id = system_file_data_gid(&fsd);;
        }

        if ( sd->identifiers & SCAN_ATTRIBUTE_OWNER_NAME )
        {
            struct passwd* pw = getpwuid ( system_file_data_uid(&fsd) );
            scan->owner_name = strdup ( pw->pw_name );
        }

        if ( sd->identifiers & SCAN_ATTRIBUTE_GROUP_NAME )
        {
            struct group* gp = getgrgid ( system_file_data_gid(&fsd) );
            scan->group_name = strdup ( gp->gr_name );
        }
        if ( sd->identifiers & SCAN_ATTRIBUTE_OWNER_PERMISSION )
        {
            char buffer[ 4 ] = { 0 };
            buffer[ 0 ] = S_IRUSR & system_file_data_mode(&fsd) ? 'R' : '-';
            buffer[ 1 ] = S_IWUSR & system_file_data_mode(&fsd) ? 'W' : '-';
            buffer[ 2 ] = S_IXUSR & system_file_data_mode(&fsd) ? 'X' : '-';
            scan->owner_permissions = strdup ( buffer );
        }
        if ( sd->identifiers & SCAN_ATTRIBUTE_GROUP_PERMISSION )
        {
            char buffer[ 4 ] = { 0 };
            buffer[ 0 ] = S_IRGRP & system_file_data_mode(&fsd) ? 'R' : '-';
            buffer[ 1 ] = S_IWGRP & system_file_data_mode(&fsd) ? 'W' : '-';
            buffer[ 2 ] = S_IXGRP & system_file_data_mode(&fsd) ? 'X' : '-';
            scan->group_permissions = strdup ( buffer );
        }

        if ( sd->identifiers & SCAN_ATTRIBUTE_MODIFY_DATE )
        {
            char buffer[ 20 ] = { 0 };
            long long modify_date = system_file_data_modify_date(&fsd);
            struct tm* time = localtime ( &modify_date );
            strftime ( buffer, sizeof ( buffer ) / sizeof ( buffer[ 0 ] ), DATE_PATTERN, time );
            scan->modify_date = strdup ( buffer );
        }

        if ( sd->identifiers & SCAN_ATTRIBUTE_CREATE_DATE )
        {
            char buffer[ 20 ] = { 0 };
            long long create_date = system_file_data_create_date(&fsd);
            struct tm* time = localtime ( &create_date );            
            strftime ( buffer, sizeof ( buffer ) / sizeof ( buffer[ 0 ] ), DATE_PATTERN, time );
            scan->create_date = strdup ( buffer );
        }

        if ( S_ISDIR ( system_file_data_mode(&fsd) ) && STACK_SIZE ( sd->directories ) <= recursion_depth + 1 )
        {
            // its a directory
            directory_information di = { 0 };
            strcpy ( di.name, filepath );
            di.pointer = opendir ( di.name );
            STACK_PUSH ( sd->directories, directory_information, di );
        }

        out.data = scan;
        out.result = PHYSICAL_RESULT_OK;

        return out;
    }

    else
    {
        // read from file
        // FIXME
        exit ( 1 );
    }
}

static struct dirent* get_next_entry ( struct scan_data* sd )
{
    while ( STACK_SIZE ( sd->directories ) > 0 )
    {
        directory_information di = STACK_PEEK ( sd->directories, directory_information );
        struct dirent* entry = readdir ( di.pointer );
        if ( entry != NULL )
        {
            return entry;
        }
        closedir ( di.pointer );
        STACK_POP ( sd->directories, directory_information );
    }
    return NULL;
}

static void delete_directories ( void* data, void* buffer )
{
    ( (void) buffer );
    directory_information di = *(directory_information*) data;
    closedir ( di.pointer );
}
