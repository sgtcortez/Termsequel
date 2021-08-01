/**
 * Should make system calls to different operating systems
*/
#ifndef __TERMSEQUEL_SYSTEM_H__
#define __TERMSEQUEL_SYSTEM_H__

#include <cstdint>
#include <unistd.h>
#include <string>
#include <vector>

namespace Termsequel {

    // Usually, will be a stat system call wrapper
    class ResultRow {
        private:
            std::uint64_t size;
            std::string name;
            std::uint8_t permissions[3];
        public:
            ResultRow(
                struct stat &stat_buffer, 
                std::string name
            );
            std::uint64_t get_size();
            std::string get_name();
    };

    class System {
        public:
            static std::vector<ResultRow *> * get_information(std::string filename);
    };

};


#endif

