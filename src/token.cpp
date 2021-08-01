#include <ostream>
#include <streambuf>
#include <string>
#include <regex>

#ifdef DEBUG
    #undef DEBUG_TOKEN
    #define DEBUG_TOKEN 1
#endif

#ifdef DEBUG_TOKEN
    #include <iostream>
#endif

#include "token.hpp"

namespace Termsequel {
    const char * const TokenMap[] = {
        "UNKNOWN",
        "COMMA", 
        "SELECT",
        "FROM",
        "WHERE",
        "NAME",
        "SIZE",
        "IDENTIFIER"
    };
}

const char * Termsequel::get_token_type_name(TOKEN_TYPE token) {
        return TokenMap[token];
}

Termsequel::TOKEN_TYPE Termsequel::get_token_type_by_name(std::string raw_value) {

    static const char * TOKEN_IDENTIFIER_REGEX = "^\"[\\w]+\"$";

    if (raw_value == "SELECT") return SELECT;
    if (raw_value == "FROM")   return FROM;
    if (raw_value == "WHERE")  return WHERE;
    if (raw_value == "NAME")   return NAME;
    if (raw_value == "SIZE")   return SIZE;
    if (raw_value == "," )     return COMMA;
    if (std::regex_match(raw_value, std::regex(TOKEN_IDENTIFIER_REGEX))) return IDENTIFIER;
    return UNKNOWN; 
}

Termsequel::Token::Token(
    Termsequel::TOKEN_TYPE type,
    std::string value
) {
    this->type = type;
    this->value = value;
}

Termsequel::TOKEN_TYPE Termsequel::Token::get_type() {
    return this->type;
}

std::string Termsequel::Token::get_value() {
    return this->value;
}

bool Termsequel::Token::is_terminal() {
    // for now, just the identifier is terminal
    return this->type == IDENTIFIER;
}