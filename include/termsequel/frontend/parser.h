#ifndef TERMSEQUEL_PARSER_H
#define TERMSEQUEL_PARSER_H

#include "node.h"

#include <stdint.h>

ast_statement_node* parse_statement ( const char* sql );

#endif // TERMSEQUEL_PARSER_H