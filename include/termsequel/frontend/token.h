#ifndef TERMSEQUEL_TOKEN_H
#define TERMSEQUEL_TOKEN_H

typedef enum token
{
    TOKEN_INVALID = 0,
    TOKEN_SELECT,
    TOKEN_INSERT,
    TOKEN_DELETE,
    TOKEN_UPDATE,
    TOKEN_FROM,
    TOKEN_WHERE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_IDENTIFIER,        // a sequence of characters without being surrounded by a double quotes
    TOKEN_COMMA,             // ,
    TOKEN_MULTIPLY,          // *
    TOKEN_NUMBER,            // any number: 1, 10, 24.23
    TOKEN_STRING,            // any string: "name", "NA", "N"
    TOKEN_LEFT_PARENTHESIS,  // (
    TOKEN_RIGHT_PARENTHESIS, // )
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_EQUAL,
    TOKEN_GREATER_THAN_OR_EQUAL,
    TOKEN_LESS_THAN_OR_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_GREATER_THAN,
    TOKEN_LESS_THAN,
    TOKEN_UPPER,
    TOKEN_LOWER,
    TOKEN_DIVISION,
    TOKEN_MODULE,
    TOKEN_MAX,
    TOKEN_MIN,
    TOKEN_AVG,
    TOKEN_BOOLEAN,
    TOKEN_EOF,
    TOKEN_AS,
    TOKEN_ORDER,
    TOKEN_BY,
    TOKEN_ASC,
    TOKEN_DESC,
    TOKEN_LIMIT,
    TOKEN_LIKE,
    TOKEN_GROUP,
    TOKEN_NOT,
    TOKEN_DATE,
} token;

typedef enum token_class
{
    TOKEN_CLASS_INVALID = 0,
    TOKEN_CLASS_KEYWORD,                    // SELECT, FROM, WHERE
    TOKEN_CLASS_SPECIAL,                    // ",", "*", "-"
    TOKEN_CLASS_BINARY_RELATIONAL_OPERATOR, // >, <, =, >=, <=, != ...
    TOKEN_CLASS_LOGICAL_OPERATOR,           // OR, AND
    TOKEN_CLASS_UNARY_ARITHMETIC_OPERATOR,  // -(NEGATE), !(FACTORIAL)
    TOKEN_CLASS_BINARY_ARITHMETIC_OPERATOR, // +, /
    TOKEN_CLASS_LITERAL,
    TOKEN_CLASS_FUNCTION,
    TOKEN_CLASS_UNARY_RELATIONAL_OPERATOR,
} token_class;

extern const char* TOKEN_NAMES[];
extern const char* TOKEN_CLASS_NAMES[];

#endif // TERMSEQUEL_TOKEN_H