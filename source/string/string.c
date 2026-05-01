#include "termsequel/string/string.h"
#include "cgeneric/array.h"

#include <stddef.h>
#include <string.h>

string string_construct ( const char* input )
{
    const size_t length = strlen ( input );
    array* array = ARRAY_CONSTRUCT ( char, length + 1 );
    memcpy ( ARRAY_CONTENT ( array, char ), input, length );
    *ARRAY_AT ( array, length, char ) = 0;
    return array;
}

void string_destroy ( string* str )
{
    array* arr = *str;
    ARRAY_DESTROY ( arr );
}

const char* string_raw ( const string str )
{
    array* arr = str;
    return ARRAY_CONTENT ( arr, char );
}