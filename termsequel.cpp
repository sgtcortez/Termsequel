#include <cstdio>


#include <cstdlib>
#include <iostream>
#include <string>

#ifdef __linux__
   #include <unistd.h>
   static constexpr const char *short_options = "h";

#elif defined (_WIN32)
#endif

#include "include/compiler.hpp"


static constexpr const char *VERSION = "0.1";

using namespace Termsequel;

void show_help(
    char *filename,
    FILE *stream  // TODO: Use c++ cout/cerr
);

int main (
    int argc,
    char **argv
) {

    int option;
    char *binary_name = argv[0];

#ifdef __linux__

    while ( (option = getopt(argc, argv, short_options)) != -1 ) {

        switch (option) {
            case 'h':
                // show help and exits
                show_help(binary_name, stdout);
                return 0;
        }
    }

    if (optind == argc) {
        // invalid. SQL wasnt provided
        show_help(binary_name, stderr);
        return 1;
    } else {
        std::string sql = argv[argc - 1];
        Compiler compiler(sql);
        compiler.execute();
    }

#elif defined(_WIN32)

   // I do not know if windows has somethig like getopt

   // no arguments were provided!
   if ( argc == 1 ) {
      show_help(binary_name, stderr);
      return 1;
   } else {
      std::string sql = argv[argc - 1];
      Compiler compiler(sql);
      compiler.execute();
   }
#endif

    return 0;
}

void show_help(
    char *filename,
    FILE *stream
) {
    std::string message =
        "Usage: %s [-h] SQL"
        "\nOptions:"
        "\n -h Show this help."
        "\nColumns available:"
        "\n NAME  Filename"
        "\n SIZE  Filesize"
        "\n OWNER File owner"
        "\nSQL instructions available"
        "\n SELECT Example: SELECT NAME FROM DIRECTORY"
        "\nVersion: %s."
        "\nCompiled at: %s:%s\n";
    fprintf(stream, message.c_str(), filename, VERSION, __DATE__, __TIME__);
}