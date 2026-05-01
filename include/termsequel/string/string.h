#ifndef TERMSEQUEL_STRING_H
#define TERMSEQUEL_STRING_H

typedef void* string;

string string_construct ( const char* input );
void string_destroy ( string* str );
const char* string_raw ( const string str );

#endif