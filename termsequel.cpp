#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <string>


static constexpr const char *short_options = "f:h";
static constexpr const char *VERSION = "0.1";

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

    while ( (option = getopt(argc, argv, short_options)) != -1 ) {
        
        switch (option) {
            case 'f':
                break;
            case 'h':
                show_help(binary_name, stdout);
                break;
        }
    }


    return 0;    
}

void show_help(
    char *filename,
    FILE *stream
) {
    std::string message = 
        "Usage: %s [-h] [-f] SQL" 
        "\nOptions:" 
        "\n -h Show this help."   
        "\n -f Read from file, instead of stdin."
        "\nSQL instructions available" 
        "\n SELECT Example: SELECT NAME FROM \"DIRECTORY\""    
        "\nVersion: %s.\n"; 
    fprintf(stream, message.c_str(), filename, VERSION);
}