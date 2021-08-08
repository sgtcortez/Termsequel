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

    enum class COMMAND_TYPE {
        LIST // For now, just the list command is available
    };

    enum class COLUMN_TYPE {
        FILENAME, // name of the file
        FILESIZE // size of the file
    };

    // Represents an execution command
    struct Command { 
        COMMAND_TYPE command; // the command to be executed
        std::vector<COLUMN_TYPE> columns; // the columns 
        std::string target; // the target file/directory
    };

    class System {
        public:

            // Executes the command, and return the list of elements
            static std::vector<std::string *> * execute(Command *command);
    };

};


#endif

