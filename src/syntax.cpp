#include "syntax.hpp"
#include "token.hpp"
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>


Termsequel::Syntax::Syntax() {
}

bool Termsequel::Syntax::analyse(Termsequel::Lexical &lexical) {

    // Given a token, what are expected token
    std::vector<TOKEN_TYPE> expectations;

    while(lexical.has_next()) {

        auto token = lexical.next_token();

        if (token.get_type() == UNKNOWN) {
            // error
            errors_list.push_back("Unknown token received!");
            break;
        }

        if (expectations.size() != 0) {
            // check expectations
            if (std::find(expectations.begin(), expectations.end(), token.get_type()) != expectations.end()) {
                expectations.clear();
                // ok, found
            } else {
                // not found
                std::string str = "Expectation failed! Received token: ";
                str.append(get_token_type_name(token.get_type()));
                str.append(" Expected: [ "); 
                for(auto element : expectations) {
                    str.append(get_token_type_name(element));
                    str.append(" ");
                }
                str.append("]");
                errors_list.push_back(str);
                break;
            }
        }

        switch (token.get_type()) {
            case SELECT:
            case COMMA:
                expectations.push_back(NAME);
                expectations.push_back(SIZE);                
                break;
            case NAME:
            case SIZE:
                expectations.push_back(COMMA);
                expectations.push_back(FROM);
                break;        
            case FROM:
                expectations.push_back(IDENTIFIER);
                break;
            default:
                // IDENTIFIER
                if (lexical.has_next()) {
                    // invalid.
                    errors_list.push_back("IDENTIFIER is the terminal!");
                }
                break;
        }

    }
    return errors_list.size() == 0;
}

std::vector<std::string> Termsequel::Syntax::get_errors() {
    return this->errors_list;
}