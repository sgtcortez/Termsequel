#ifndef TERMSEQUEL_LEXER_H
#define TERMSEQUEL_LEXER_H

#include <stdbool.h>
#include <stdint.h>

#include "token.h"

typedef struct lexer lexer;
typedef struct lexeme
{
    uint16_t start_index;
    uint16_t end_index;
    token token_type;
    token_class token_class;
    char value[];
} lexeme;

lexer* lexer_init ( const char* input );
bool lexer_has_next ( const lexer* lexer );
lexeme* lexer_next ( lexer* lexer );
void lexer_puke ( lexer* lexer, lexeme* lexeme );
void lexer_free ( lexer** lexer );
token lexeme_token ( const lexeme* lexeme );

#endif // TERMSEQUEL_LEXER_H