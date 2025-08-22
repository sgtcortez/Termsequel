#ifndef __TERMSEQUEL_LOG__
#define __TERMSEQUEL_LOG__

#include <iostream>

#define __FILENAME__ ( __builtin_strrchr ( __FILE__, '/' ) ? __builtin_strrchr ( __FILE__, '/' ) + 1 : __FILE__ ) // only show filename and not it's path (less clutter)

#ifdef DEBUG
#define LOG_DEBUG( ... ) std::cout << "[DEBUG] " << __FILENAME__ << "@" << __FUNCTION__ << ":" << __LINE__ << " - " << __VA_ARGS__ << std::endl;
#else
#define LOG_DEBUG( ... )                                                                                                                                                                               \
    do                                                                                                                                                                                                 \
        {                                                                                                                                                                                              \
            ;                                                                                                                                                                                          \
    } while ( 0 )
#endif

#define LOG_ERROR( ... ) std::cerr << "[ERROR] " << __FILENAME__ << "@" << __FUNCTION__ << ":" << __LINE__ << " - " << __VA_ARGS__ << std::endl;

#endif