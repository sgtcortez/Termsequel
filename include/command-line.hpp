#ifndef TERMSEQUEL_COMMAND_LINE_H__
#define TERMSEQUEL_COMMAND_LINE_H__

#include <cstdint>
#include <vector>
#include <string>

namespace Termsequel {

   class CommandLineParser {
      private:
         std::uint8_t current;
         std::vector<std::string> arguments;
      public:
         CommandLineParser (
            char **argv,
            int argc
         );
         bool has_next ();
         std::string parse ();
   };

};






#endif