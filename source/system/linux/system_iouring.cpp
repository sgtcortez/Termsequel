#if defined __linux__ //&& defined(__use_iouring__)

#include "termsequel/log.hpp"
#include "termsequel/system/system.hpp"

#include <array>
#include <cstring>

#include <chrono>
#include <dirent.h> /* Defines DT_* constants */
#include <sys/syscall.h>

using namespace termsequel;
using namespace termsequel::system;
using namespace std;
using namespace chrono_literals;

/**
 * This is the implementation for Linux using IO_URING.
 * https://man.archlinux.org/man/io_uring.7.en
 *
 * Since, using the pure IOURING system calls are a bit hard, we can leverage from liburing.
 * https://github.com/axboe/liburing
 */
#include <liburing.h>

struct Request
{
    enum class Type : uint8_t
    {
        OPEN_DIRECTORY = 1,
        CLOSE          = 2,
        STAT           = 3,
        // https://en.wikipedia.org/wiki/List_of_file_signatures
        READ      = 4,
        OPEN_FILE = 5,
        TIMEOUT   = 6,
    } type;

    void *buffer;

    void *get_syscall_buffer () const
    {
        switch ( type )
            {
            case Request::Type::STAT:
                return this->buffer;
            default:
                return nullptr;
            }
    }

    void *get_user_buffer () const
    {
        switch ( type )
            {
            case Request::Type::STAT:
                return static_cast<char *> ( this->buffer ) + sizeof ( struct statx );
            default:
                return this->buffer;
            }
    }

    string name ()
    {
        static const char *names[] = { "", "OPEN_DIRECTORY", "CLOSE", "STAT", "READ", "OPEN_FILE", "TIMEOUT" };
        return names[ static_cast<int> ( this->type ) ];
    }

    // Allocation/Deallocation should be done with C memory functions
    Request ()  = delete;
    ~Request () = delete;

    static Request *make_request_by_type ( Type type, void *userdata, size_t userdata_size )
    {
        size_t datatype_size = sizeof ( Request );
        size_t syscall_data_size;

        switch ( type )
            {
            case Request::Type::STAT:
                syscall_data_size = sizeof ( struct statx );
                break;
            default:
                syscall_data_size = 0;
            }

        size_t size     = datatype_size + syscall_data_size + userdata_size;
        void  *buffer   = calloc ( size, sizeof ( char ) );
        auto   request  = static_cast<Request *> ( buffer );
        request->type   = type;
        request->buffer = static_cast<char *> ( buffer ) + datatype_size;
        if ( userdata_size > 0 )
            {
                memcpy ( static_cast<char *> ( buffer ) + ( size - userdata_size ), userdata, userdata_size );
            }
        return request;
    }
};

void            request_statx ( const string &location, struct io_uring &uring );
void            request_open_directory ( const string &location, struct io_uring &uring );
void            request_open_file ( const string &location, struct io_uring &uring );
void            request_close ( int32_t fd, struct io_uring &uring );
void            request_timeout ( struct io_uring &uring );
void            list_directories ( int32_t directory_fd, const string &location, struct io_uring &uring );
FileInformation map ( const Request *request );

System::System ( on_file_callback callback ) : interested_fields_mask ( STATX_BTIME | STATX_MTIME | STATX_CTIME ), callback ( callback )
{
    struct io_uring *uring = new io_uring;
    poll                   = uring;
    if ( io_uring_queue_init ( 10000, uring, 0 ) < 0 )
        {
            LOG_ERROR ( "Could not initialize uring!" );
            throw "Error";
        }
}

System::~System ()
{
    struct io_uring *ring = static_cast<struct io_uring *> ( this->poll );
    io_uring_queue_exit ( ring );
}

void System::query ( const string &location )
{
    /**
     * This method will be called once, and will start the IO uring.
     * We request information for the file, then, if file is a directory, we list the files for the directory
     * if file its a regular file, then we just read the file content and search
     */
    request_timeout ( ( *static_cast<io_uring *> ( this->poll ) ) );
    request_open_directory ( location, ( *static_cast<io_uring *> ( this->poll ) ) );
    this->handle_io ();
}

void System::handle_io ()
{
    struct io_uring     *ring = static_cast<struct io_uring *> ( this->poll );
    struct io_uring_cqe *completion_queue_entry;
    bool                 running = true;
    while ( running )
        {
            if ( io_uring_wait_cqe ( ring, &completion_queue_entry ) < 0 )
                {
                    LOG_ERROR ( "Error while waiting for completed submissions!" );
                    throw "Error";
                }

            auto request = (Request *)completion_queue_entry->user_data;
            io_uring_cqe_seen ( ring, completion_queue_entry );
            if ( completion_queue_entry->res < 0 )
                {

                    if ( request->type == Request::Type::TIMEOUT )
                        {
                            // no request to iouring happened in the timeout defined
                            // so we break
                            LOG_ERROR ( "Timeout!" );
                            running = false;
                            goto CLEANUP;
                        }

                    /*
                     * IO_URING will return the errno value in cqe->res.
                     * The errno is always positive, however, since we use negative number to indicate an error.
                     * This means, that: cqe->res = - ERRNO
                     * We can use the absolute function, to ensure that we will always have the positive value.
                     */
                    auto error_code = abs ( completion_queue_entry->res );
                    LOG_ERROR ( "System call failed with \"" + to_string ( error_code ) + " - " + strerror ( error_code ) + "\". Systemcall type: " + request->name () );
                    goto CLEANUP;
                }

            switch ( request->type )
                {
                case Request::Type::OPEN_DIRECTORY:
                    {
                        list_directories ( completion_queue_entry->res, string ( static_cast<char *> ( request->get_user_buffer () ) ), ( *static_cast<io_uring *> ( this->poll ) ) );
                        break;
                    }
                case Request::Type::STAT:
                    {
                        callback ( map ( request ) );
                        break;
                    }
                case Request::Type::TIMEOUT:
                    request_timeout ( ( *static_cast<io_uring *> ( this->poll ) ) );
                    break;
                default:
                    break;
                }

        CLEANUP:
            free ( request );
        }
}

void request_statx ( const string &location, struct io_uring &uring )
{
    auto request                = Request::make_request_by_type ( Request::Type::STAT, (void *)( location.c_str () ), strlen ( location.c_str () ) + 1 );
    auto submission_queue_entry = io_uring_get_sqe ( &uring );
    // https://man7.org/linux/man-pages/man2/statx.2.html
    // https://man7.org/linux/man-pages/man3/io_uring_prep_statx.3.html
    // man 3 io_uring_prep_statx
    io_uring_prep_statx ( submission_queue_entry, AT_FDCWD, location.c_str (), 0, STATX_BTIME | STATX_MTIME | STATX_CTIME, static_cast<struct statx *> ( request->buffer ) );
    io_uring_sqe_set_data ( submission_queue_entry, request );
    io_uring_submit ( &uring );
}

void request_open_directory ( const string &location, struct io_uring &uring )
{
    auto       request                = Request::make_request_by_type ( Request::Type::OPEN_DIRECTORY, (void *)( location.c_str () ), strlen ( location.c_str () ) + 1 );
    auto       submission_queue_entry = io_uring_get_sqe ( &uring );
    const auto flags                  = O_DIRECTORY | O_RDONLY | O_NOATIME;
    io_uring_prep_openat ( submission_queue_entry, AT_FDCWD, location.c_str (), flags, 0 );
    io_uring_sqe_set_data ( submission_queue_entry, request );
    io_uring_submit ( &uring );
}

void request_close ( int32_t fd, struct io_uring &uring )
{
    auto submission_queue_entry = io_uring_get_sqe ( &uring );
    io_uring_prep_close ( submission_queue_entry, fd );
    io_uring_sqe_set_data ( submission_queue_entry, nullptr );
    // we are not interested in the response of the close system call
    io_uring_sqe_set_flags ( submission_queue_entry, IOSQE_CQE_SKIP_SUCCESS );
    io_uring_submit ( &uring );
}

void list_directories ( int32_t directory_fd, const string &location, struct io_uring &uring )
{
    // https://man7.org/linux/man-pages/man2/getdents.2.html
    struct linux_dirent
    {
        unsigned long  d_ino;
        off_t          d_off;
        unsigned short d_reclen;
        char           d_name[];
    };

    char buffer[ 1024 ];
    for ( ;; )
        {
            const auto entries = syscall ( SYS_getdents, directory_fd, buffer, 1024 );
            if ( entries < 0 )
                {
                    LOG_ERROR ( "Could not get directories!" << strerror ( errno ) );
                    return;
                }
            if ( entries == 0 )
                {
                    // No more directories to read
                    break;
                }
            for ( int index = 0; index < entries; )
                {
                    struct linux_dirent *d      = (struct linux_dirent *)( buffer + index );
                    char                 d_type = *( buffer + index + d->d_reclen - 1 );
                    index += d->d_reclen;
                    if ( strncmp ( d->d_name, ".", 1 ) == 0 || strncmp ( d->d_name, "..", 2 ) == 0 )
                        {
                            // ignoring pseudo directories
                            continue;
                        }
                    if ( d_type == DT_DIR )
                        {
                            // Will open this file to be listed
                            const string loca = location + "/" + d->d_name;
                            request_open_directory ( loca, uring );
                        }
                    if ( d_type == DT_REG )
                        {
                            // a regular file
                            const string loca = location + "/" + d->d_name;
                            request_statx ( loca, uring );
                        }
                }
        }
    request_close ( directory_fd, uring );
}

void request_timeout ( struct io_uring &uring )
{
    // https://man7.org/linux/man-pages/man3/io_uring_prep_timeout.3.html
    struct __kernel_timespec ts;
    ts.tv_sec  = 0;
    ts.tv_nsec = 1ms / 1ns;

    auto submission_queue_entry = io_uring_get_sqe ( &uring );

    Request *timeout_request = Request::make_request_by_type ( Request::Type::TIMEOUT, nullptr, 0 );
    io_uring_prep_timeout ( submission_queue_entry, &ts, 1, IORING_TIMEOUT_REALTIME );
    io_uring_sqe_set_data ( submission_queue_entry, timeout_request );
    io_uring_submit ( &uring );
}

FileInformation map ( const Request *request )
{
    FileInformation file;

    struct statx *stat_buffer = static_cast<struct statx *> ( request->get_syscall_buffer () );
    string        file_name   = static_cast<char *> ( request->get_user_buffer () );

    file.size          = stat_buffer->stx_size;
    file.relative_path = file_name;

    {
        // YYYY-MM-DD HH:MM:SS
        array<char, 20> buffer = { 0 };
        const time_t    t      = stat_buffer->stx_btime.tv_sec;
        auto            date   = localtime ( &t );
        strftime ( buffer.data (), buffer.max_size (), "%Y-%m-%dT%H:%M:%S", date );
        file.creation = buffer.data ();
    }

    {
        auto last_slash = file_name.find_last_of ( '/' );
        if ( last_slash == string::npos )
            {
                last_slash = -1;
            }
        file.name = file_name.substr ( last_slash + 1 );
    }
    return file;
}

#endif