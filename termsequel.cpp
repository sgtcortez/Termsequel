#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <limits.h>

#include "include/compiler.hpp"
#include "include/command-line.hpp"

#include "termsequel.hpp"

using namespace Termsequel;

void show_help(
    char *filename,
    FILE *stream  // TODO: Use c++ cout/cerr
);

int main (
    int argc,
    char **argv
) {

   char *binary_name = argv[0];
   struct SystemCommand *command;
   bool ok = false;
   std::uint16_t max_depth = USHRT_MAX;

   // the last argument, will always be the SQL instruction ...
   std::string sql = argv[argc - 1];

   // ignores the binary name ...
   CommandLineParser parser(argv + 1, argc - 1);
   while (parser.has_next()) {
      std::string result = parser.parse();
      if ( result == "-h" || result == "--help" ) {
         show_help(binary_name, stdout);
         return 0;
      } else if (result == "-n" || result == "--max-depth") {
         if (parser.has_next()) {
            // atoi returns 0 if string is not a number
            int value = std::atoi(parser.parse().c_str());
            if (value <= 0) {
               // invalid. Should be positive ...
               std::cerr << "Max depth should have a value greater than 0 ..." << std::endl;
               show_help(binary_name, stderr);
               return 1;
            }
            max_depth = (std::uint16_t) value;
         } else {
            // expected value ...
            std::cerr << "The option: \"" << result << "\" requires an value. I.e: --max-depth 3 ..." << std::endl;
            show_help(binary_name, stderr);
            return 1;
         }


      } else if ( result[0] == '-' ) {
         std::cerr << "Invalid option: \"" << result << "\" ..." << std::endl;
         show_help(binary_name, stderr);
         return 1;
      }
   }

   Compiler compiler(sql);
   command = compiler.compile();
   if (command) ok = System::execute(command, max_depth);
   else return 1;

   if (command->conditions) {
      // Use LVALUE to avoid copying ...
      for (const auto &condition : command->conditions->conditions) delete condition;
      delete command->conditions;
   }
   // manually deletes command.
   // We should use unique_ptr, but, I was facing issues with copy constructor and things like this,
   // then, I decided for now, to manually delete
   delete command;

   int exit_status = ok ? 0 : 1;
   return exit_status;
}

void show_help(
    char *filename,
    FILE *stream
) {
    std::string message =
        "Usage: %s [Options] SQL"
        "\nOptions:"
        "\n -h, --help      Show this help and exit."
        "\n -n, --max-depth Set the maximum depth when going recursively."
        "\nColumns available:"
        "\n NAME               Filename"
        "\n SIZE               Filesize"
        "\n OWNER              File owner"
        "\n FILE_TYPE          Filetype"
        "\n LEVEL              Depth level"
        "\n OWNER_PERMISSIONS  Owner permissions"
#ifdef __linux__
        "\n GROUP_PERMISSIONS  Group permissions"
        "\n OTHERS_PERMISSIONS Others permissions"
#endif
        "\n LAST_MODIFICATION  The last modification of the file"
        "\n CREATION_DATE      The creation date of the file"
        "\n RELATIVE_PATH      The relative path of the file"
        "\n ABSOLUTE_PATH      The absolute path of the file"
        "\nSQL instructions available"
        "\n SELECT Example: SELECT NAME FROM DIRECTORY"
         "\nCompiled at: %s:%s"
#if defined(TERMSEQUEL_VERSION_PATCH) && (TERMSEQUEL_VERSION_PATCH + 0 > 0)
         "\nVersion: %d.%d.%d\n";
         fprintf(stream, message.c_str(), filename,  __DATE__, __TIME__, TERMSEQUEL_VERSION_MAJOR, TERMSEQUEL_VERSION_MINOR, TERMSEQUEL_VERSION_PATCH);
#else
         "\nVersion: %d.%d\n";
         fprintf(stream, message.c_str(), filename, __DATE__, __TIME__, TERMSEQUEL_VERSION_MAJOR, TERMSEQUEL_VERSION_MINOR );
#endif
}