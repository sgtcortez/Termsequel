#include "token.hpp"
#include <cstdint>
#include <regex>
#include <string>
#include <utility>

#ifdef DEBUG
    #undef DEBUG_LEXICAL
    #define DEBUG_LEXICAL 1
#endif

#ifdef DEBUG_LEXICAL
    #include <iostream>
#endif


#include "lexical.hpp"

namespace Termsequel {

    // Read the user input, and, split the raw string input, into an array of values splited by whitespace.
    static constexpr const char * const NON_WHITESPACES_REGEX = "(\\S)*(\\w+|,)(\\S)*";
}

Termsequel::Lexical::Lexical(std::string raw_input) {

    this->raw_input = raw_input;
    this->current_index = 0;

    const std::regex word_regex(NON_WHITESPACES_REGEX);

    const auto words_begin = std::sregex_iterator(this->raw_input.begin(), this->raw_input.end(), word_regex);
    const auto words_end = std::sregex_iterator();

    for (auto i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::string match_str = match.str();
        const auto token_type = get_token_type_by_name(match_str);

        if (token_type == IDENTIFIER) {
            std::string tmp = match_str;
            // TODO: Fix it, find a way to remove the double quotes without the need to duplicate strings
            tmp.pop_back(); // remove the right double quote
            const char *cstring = tmp.data()+1; // remove the start double quote, a very hacky way to do this. 
            tokens.push_back(Token(token_type, cstring));
        } else if ( token_type == UNKNOWN ) {
            // should reject here ...
        } else {
            tokens.push_back(Token(token_type, ""));
        }

        #ifdef DEBUG_LEXICAL
                std::cout << "Found match: " <<  match_str << " Token type: " << get_token_type_name(token_type) << std::endl;
        #endif
    }

}

bool Termsequel::Lexical::has_next() {
    return tokens.size() > current_index;
}

Termsequel::Token Termsequel::Lexical::next_token() {
    return tokens[current_index++];
}


