#ifndef __TERMSEQUEL_COMPILER_H__
#define __TERMSEQUEL_COMPILER_H__

/**
 * This class, should get the raw input, and produce a sequence of commands to be executable on the host.
 * --------------------------------------------------------------------------------------------------------
 * For example, the SQL instuction: SELECT NAME FROM "DIRECTORY" means: Get all file/directory names from directory.
 * If the host runs linux, it should provide linux commands(syscalls mostly) to be able to execute.
 * If the host runs Windows, it should provide Windows commands to be able to execute.
*/


#include <string>
#include "system.hpp"

namespace Termsequel {

   class Compiler {
      private:
         std::string raw_input;
      public:
         Compiler(std::string raw_input);
         void execute();
   };


};

#endif /* __TERMSEQUEL_COMPILER_H__ */