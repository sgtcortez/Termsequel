#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <ostream>
#include <string>
#include <limits.h>

#include "include/compiler.hpp"
#include "include/command-line.hpp"

#include "termsequel.hpp"

using namespace Termsequel;

bool human_readable = false;
bool use_decimal_unit = true;

void show_help(
    char *filename,
    const bool error
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
      if ( result == "--help" ) {
         show_help(binary_name, false);
         return 0;
      } else if (result == "-h" || result == "--human-readable") {
         human_readable = true;
      } else if (result == "-n" || result == "--max-depth") {
         if (parser.has_next()) {
            // atoi returns 0 if string is not a number
            int value = std::atoi(parser.parse().c_str());
            if (value <= 0) {
               // invalid. Should be positive ...
               std::cerr << "Max depth should have a value greater than 0 ..." << std::endl;
               show_help(binary_name, true);
               return 1;
            }
            max_depth = (std::uint16_t) value;
         } else {
            // expected value ...
            std::cerr << "The option: \"" << result << "\" requires an value. I.e: --max-depth 3 ..." << std::endl;
            show_help(binary_name, true);
            return 1;
         }
      } else if ( result == "-b" || result == "--base" ) {
         if (parser.has_next()) {
            // atoi returns 0 if string is not a number
            int value = std::atoi(parser.parse().c_str());
            if (value == 2) {
               use_decimal_unit = false;
            } else if (value == 10) {
               use_decimal_unit = true;
            } else {
            // invalid. Should be positive ...
               std::cerr << "Base should be 2 or 10!" << std::endl;
               show_help(binary_name, true);
               return 1;
            }
         }

      } else if ( result[0] == '-' ) {
         std::cerr << "Invalid option: \"" << result << "\" ..." << std::endl;
         show_help(binary_name, true);
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
    const bool error
) {
   std::ostream & output = error ? std::cerr : std::cout;
   output << "Usage: " << filename <<  " [--help] [-n, --max-depth] [-h, --human-readable] [-b, --base] SQL";
   output << "\nOptions:";
   output << "\n     --help           Show this help and exit.";
   output << "\n -n, --max-depth      Set the maximum depth when going recursively.";
   output << "\n -h, --human-readable Display information in a human readable format.";
   output << "\n -b, --base           The base which the values are shown. Default is 10(decimal).";
   output << "\nColumns available:";
   output << "\n NAME               Filename";
   output << "\n SIZE               Filesize";
   output << "\n OWNER              File owner";
#ifdef __linux__
   output << "\n GROUP              Group owner";
#endif
   output << "\n FILE_TYPE          Filetype";
   output << "\n LEVEL              Depth level";
   output << "\n OWNER_PERMISSIONS  Owner permissions";
#ifdef __linux__
   output << "\n GROUP_PERMISSIONS  Group permissions";
   output << "\n OTHERS_PERMISSIONS Others permissions";
#endif
   output << "\n LAST_MODIFICATION  The last modification of the file";
   output << "\n CREATION_DATE      The creation date of the file";
   output << "\n RELATIVE_PATH      The relative path of the file";
   output << "\n ABSOLUTE_PATH      The absolute path of the file";
   output << "\nSQL instructions available";
   output << "\n SELECT Example: SELECT NAME FROM DIRECTORY";
   output << "\nCompiled at: " << __DATE__ << " " << __TIME__;
#if defined(TERMSEQUEL_VERSION_PATCH) && (TERMSEQUEL_VERSION_PATCH + 0 > 0)
   output << "\nVersion: " << TERMSEQUEL_VERSION_MAJOR << "." << TERMSEQUEL_VERSION_MINOR << "." << TERMSEQUEL_VERSION_PATCH;
#else
   output << "\nVersion: " << TERMSEQUEL_VERSION_MAJOR << "." << TERMSEQUEL_VERSION_MINOR;
#endif
   output << std::endl;
}