#include <cstdio>


#include <cstdlib>
#include <iostream>
#include <string>

#ifdef __linux__
   #include <unistd.h>
   static constexpr const char *short_options = "hv";

#elif defined (_WIN32)
#endif

#include "include/compiler.hpp"
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

#ifdef __linux__

   int option;

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
        "\nSQL instructions available"
        "\n SELECT Example: SELECT NAME FROM DIRECTORY"
         "\nCompiled at: %s:%s"
#if defined(TERMSEQUEL_VERSION_PATCH) && (TERMSEQUEL_VERSION_PATCH > 0)
         "\nVersion: %d.%d.%d\n";
         fprintf(stream, message.c_str(), filename,  __DATE__, __TIME__, TERMSEQUEL_VERSION_MAJOR, TERMSEQUEL_VERSION_MINOR, TERMSEQUEL_VERSION_PATCH);
#else
         "\nVersion: %d.%d\n";
         fprintf(stream, message.c_str(), filename, __DATE__, __TIME__, TERMSEQUEL_VERSION_MAJOR, TERMSEQUEL_VERSION_MINOR );
#endif
}