#include "termsequel/frontend/lexer.h"
#include "termsequel/frontend/token.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct lexer
{
    uint16_t index;
    uint16_t value_length;
    char value[];
};

struct keyword_information
{
    token type;
    token_class class;
    char* name;
};

static struct keyword_information keywords[] = {
    {
        .name = "SELECT",
        .type = TOKEN_SELECT,
        .class = TOKEN_CLASS_KEYWORD,
    },
    {
        .name = "INSERT",
        .type = TOKEN_INSERT,
        .class = TOKEN_CLASS_KEYWORD,
    },
    {
        .name = "DELETE",
        .type = TOKEN_DELETE,
        .class = TOKEN_CLASS_KEYWORD,
    },
    {
        .name = "UPDATE",
        .type = TOKEN_UPDATE,
        .class = TOKEN_CLASS_KEYWORD,
    },
    {
        .name = "FROM",
        .type = TOKEN_FROM,
        .class = TOKEN_CLASS_KEYWORD,
    },
    {
        .name = "WHERE",
        .type = TOKEN_WHERE,
        .class = TOKEN_CLASS_KEYWORD,
    },
    {
        .name = "AND",
        .type = TOKEN_AND,
        .class = TOKEN_CLASS_LOGICAL_OPERATOR,
    },
    {
        .name = "OR",
        .type = TOKEN_OR,
        .class = TOKEN_CLASS_LOGICAL_OPERATOR,
    },
    { .name = "(", .class = TOKEN_CLASS_SPECIAL, .type = TOKEN_LEFT_PARENTHESIS },
    { .name = ")", .class = TOKEN_CLASS_SPECIAL, .type = TOKEN_RIGHT_PARENTHESIS },
    { .name = "+", .class = TOKEN_CLASS_BINARY_ARITHMETIC_OPERATOR, .type = TOKEN_PLUS },
    { .name = "-", .class = TOKEN_CLASS_SPECIAL, .type = TOKEN_MINUS },
    {
        .name = "*",
        .class = TOKEN_CLASS_BINARY_ARITHMETIC_OPERATOR,
        .type = TOKEN_MULTIPLY,
    },
    {
        .name = ",",
        .class = TOKEN_CLASS_SPECIAL,
        .type = TOKEN_COMMA,
    },
    {
        .name = "/",
        .class = TOKEN_CLASS_BINARY_ARITHMETIC_OPERATOR,
        .type = TOKEN_DIVISION,
    },
    {
        .name = "=",
        .class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR,
        .type = TOKEN_EQUAL,
    },
    { .name = "UPPER", .class = TOKEN_CLASS_FUNCTION, .type = TOKEN_UPPER },
    {
        .name = "LOWER",
        .class = TOKEN_CLASS_FUNCTION,
        .type = TOKEN_LOWER,
    },
    { .name = "MAX", .class = TOKEN_CLASS_FUNCTION, .type = TOKEN_MAX },
    { .name = "MIN", .class = TOKEN_CLASS_FUNCTION, .type = TOKEN_MIN },
    { .name = "AVG", .class = TOKEN_CLASS_FUNCTION, .type = TOKEN_AVG },
    {
        .name = ">=",
        .class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR,
        .type = TOKEN_GREATER_THAN_OR_EQUAL,
    },
    {
        .name = "<=",
        .class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR,
        .type = TOKEN_LESS_THAN_OR_EQUAL,
    },
    {
        .name = ">",
        .class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR,
        .type = TOKEN_GREATER_THAN,
    },
    {
        .name = "<",
        .class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR,
        .type = TOKEN_LESS_THAN,
    },
    {
        .name = "TRUE",
        .class = TOKEN_CLASS_LITERAL,
        .type = TOKEN_BOOLEAN,
    },
    {
        .name = "FALSE",
        .class = TOKEN_CLASS_LITERAL,
        .type = TOKEN_BOOLEAN,
    },
    {
        .name = "AS",
        .class = TOKEN_CLASS_KEYWORD,
        .type = TOKEN_AS,
    },
    {
        .name = "ORDER",
        .class = TOKEN_CLASS_KEYWORD,
        .type = TOKEN_ORDER,
    },
    {
        .name = "BY",
        .class = TOKEN_CLASS_KEYWORD,
        .type = TOKEN_BY,
    },
    { .name = "ASC", .class = TOKEN_CLASS_KEYWORD, .type = TOKEN_ASC },
    { .name = "DESC", .class = TOKEN_CLASS_KEYWORD, .type = TOKEN_DESC },
    {
        .name = "LIMIT",
        .class = TOKEN_CLASS_KEYWORD,
        .type = TOKEN_LIMIT,
    },
    {
        .name = "LIKE",
        .class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR,
        .type = TOKEN_LIKE,
    },
    {
        .name = "GROUP",
        .class = TOKEN_CLASS_KEYWORD,
        .type = TOKEN_GROUP,
    },
    {
        .name = "NOT",
        .class = TOKEN_CLASS_UNARY_RELATIONAL_OPERATOR,
        .type = TOKEN_NOT,
    },
    {
        .name = "DATE",
        .class = TOKEN_CLASS_FUNCTION,
        .type = TOKEN_DATE,
    },
};

uint16_t read_while_inbounds ( char* data, int start_bound, int end_bound, int skip_if );
uint16_t read_keyword_or_identifier ( char* data );

int32_t read_until ( char* data, char until );
lexeme* parse_number ( const char* input, uint16_t length );
lexeme* parse_identifier_or_keyword ( const char* input, uint16_t length );
lexeme* parse_string ( const char* input, uint16_t size );

lexer* lexer_init ( const char* input )
{
    const size_t required_length = sizeof ( lexer ) + strlen ( input );
    lexer* lexer = malloc ( required_length );
    lexer->index = 0;
    lexer->value_length = strlen ( input );
    memcpy ( lexer->value, input, lexer->value_length );
    return lexer;
}

bool lexer_has_next ( const lexer* lexer )
{
    if ( lexer->index >= lexer->value_length )
    {
        return false;
    }
    return true;
}

lexeme* lexer_next ( lexer* lexer )
{
    uint16_t start_index = lexer->index;
    lexeme* lex = NULL;
    while ( start_index < lexer->value_length )
    {
        char c = lexer->value[ start_index ];
        if ( c >= '0' && c <= '9' )
        {
            int size = read_while_inbounds ( lexer->value + start_index, '0', '9', '.' );
            lex = parse_number ( lexer->value + start_index, size );
            start_index += size;
            break;
        }
        else if ( ( c >= 'A' && c <= 'Z' ) || ( c >= 'a' && c <= 'z' ) )
        {
            int size = read_keyword_or_identifier ( lexer->value + start_index );
            lex = parse_identifier_or_keyword ( lexer->value + start_index, size );
            start_index += size;
            break;
        }
        else if ( c == '"' )
        {
            int size = read_until ( lexer->value + start_index + 1, '"' );
            if ( size == -1 )
            {
                // not found
                fprintf ( stderr,
                          "LEXER error: Unclosed \""
                          "\"" );
                exit ( 10 );
            }
            // to ignore the double quotes
            lex = parse_string ( lexer->value + start_index + 1, size );
            // it's safe to do this, cause the next element, will be 0 if we reached the
            // end of the string
            start_index += 1 + size + 1;
            break;
        }
        else if ( c == '(' || c == ')' || c == ',' || c == '*' || c == '+' || c == '-' || c == '/' )
        {
            lex = malloc ( sizeof ( lexeme ) );
            for ( size_t i = 0; i < sizeof ( keywords ) / sizeof ( keywords[ 0 ] ); i++ )
            {
                if ( strncmp ( keywords[ i ].name, lexer->value + start_index, 1 ) == 0 )
                {
                    lex->token_class = keywords[ i ].class;
                    lex->token_type = keywords[ i ].type;
                    break;
                }
            }
            start_index++;
            break;
        }
        else if ( lexer->value[ start_index ] == '>' && lexer->value[ start_index + 1 ] == '=' )
        {
            lex = malloc ( sizeof ( lexeme ) );
            lex->token_class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR;
            lex->token_type = TOKEN_GREATER_THAN_OR_EQUAL;
            start_index += 2;
            break;
        }
        else if ( lexer->value[ start_index ] == '!' && lexer->value[ start_index + 1 ] == '=' )
        {
            lex = malloc ( sizeof ( lexeme ) );
            lex->token_class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR;
            lex->token_type = TOKEN_NOT_EQUAL;
            start_index += 2;
            break;
        }
        else if ( lexer->value[ start_index ] == '=' && lexer->value[ start_index + 1 ] == '<' )
        {
            lex = malloc ( sizeof ( lexeme ) );
            lex->token_class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR;
            lex->token_type = TOKEN_LESS_THAN_OR_EQUAL;
            start_index += 2;
            break;
        }
        else if ( lexer->value[ start_index ] == '=' )
        {
            lex = malloc ( sizeof ( lexeme ) );
            lex->token_class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR;
            lex->token_type = TOKEN_EQUAL;
            start_index += 1;
            break;
        }
        else if ( lexer->value[ start_index ] == '>' )
        {
            lex = malloc ( sizeof ( lexeme ) );
            lex->token_class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR;
            lex->token_type = TOKEN_GREATER_THAN;
            start_index += 1;
            break;
        }
        else if ( lexer->value[ start_index ] == '<' )
        {
            lex = malloc ( sizeof ( lexeme ) );
            lex->token_class = TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR;
            lex->token_type = TOKEN_LESS_THAN;
            start_index += 1;
            break;
        }
        else if ( c == ' ' )
        {
            start_index++;
            continue;
        }
        else
        {
            fprintf ( stderr, "LEXER error: Unexpected char: \"%c\"", c );
            exit ( 40 );
        }
    }
    lexer->index = start_index;
    return lex;
}

uint16_t read_while_inbounds ( char* data, int start_bound, int end_bound, int skip_if )
{
    uint16_t size = 0;
    while ( *data )
    {
        char c = *data;
        if ( ( c >= start_bound && c <= end_bound ) || c == skip_if )
        {
            size++;
            data++;
            continue;
        }
        break;
    }
    return size;
}

lexeme* parse_number ( const char* input, uint16_t length )
{
    size_t required_size = sizeof ( struct lexeme ) + length + 1;
    lexeme* lex = malloc ( required_size );
    memset ( lex, 0, required_size );
    lex->token_type = TOKEN_NUMBER;
    lex->token_class = TOKEN_CLASS_LITERAL;
    strncpy ( lex->value, input, length );
    return lex;
}

lexeme* parse_identifier_or_keyword ( const char* input, uint16_t length )
{
    token type = TOKEN_IDENTIFIER;
    token_class class = TOKEN_CLASS_LITERAL;

    for ( size_t i = 0; i < sizeof ( keywords ) / sizeof ( keywords[ 0 ] ); i++ )
    {
        if ( strncasecmp ( keywords[ i ].name, input, length ) == 0 )
        {
            class = keywords[ i ].class;
            type = keywords[ i ].type;
            break;
        }
    }
    lexeme* lex;
    if ( type != TOKEN_IDENTIFIER )
    {
        lex = malloc ( sizeof ( struct lexeme ) );
        memset ( lex, 0, sizeof ( struct lexeme ) );
    }
    else
    {
        size_t required_size = sizeof ( struct lexeme ) + length + 1;
        lex = malloc ( required_size );
        memset ( lex, 0, required_size );
        strncpy ( lex->value, input, length );
    }
    lex->token_type = type;
    lex->token_class = class;
    return lex;
}

int32_t read_until ( char* data, char until )
{
    uint16_t size = 0;
    bool found = false;
    while ( *data )
    {
        if ( *data == until )
        {
            found = true;
            break;
        }
        size++;
        data++;
    }
    if ( !found )
    {
        return -1;
    }
    return size;
}

lexeme* parse_string ( const char* input, uint16_t size )
{
    const size_t required_size = sizeof ( struct lexeme ) + size + 1;
    lexeme* lex = malloc ( required_size );
    memset ( lex, 0, required_size );
    strncpy ( lex->value, input, size );
    lex->token_type = TOKEN_STRING;
    lex->token_class = TOKEN_CLASS_LITERAL;
    return lex;
}

uint16_t read_keyword_or_identifier ( char* data )
{
    uint16_t size = 0;
    while ( *data )
    {
        char c = *data;
        if ( ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || c == '_' )
        {
            size++;
            data++;
            continue;
        }
        break;
    }
    return size;
}