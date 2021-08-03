#include <algorithm>
#include <cstdint>
#include <map>
#include <regex>
#include <sstream>
#include <streambuf>
#include <string>
#include <utility>
#include <iostream>
#include <vector>

#include "compiler.hpp"

// if debug all flag is set
#ifdef DEBUG 
#   undef DEBUG_COMPILER
#   define DEBUG_COMPILER 1
#endif

#ifdef DEBUG_COMPILER
#   include <iostream>
#endif

namespace Termsequel {
    
    enum TOKEN_TYPE {
        UNKNOWN,    // Invalid. Unreconized token
        COMMA,      // Comma to separate columns 
        SELECT,     // SQL SELECT KEYWORD
        FROM,       // SQL FROM KEYWORD
        WHERE,      // SQL WHERE KEYWORD
        NAME,       // NAME column. 
        SIZE,       // SIZE column.
        IDENTIFIER  // An identifer. A directory name, file name for example
    };

    static constexpr const char * const TokenMap[] = {
        "UNKNOWN",
        "COMMA", 
        "SELECT",
        "FROM",
        "WHERE",
        "NAME",
        "SIZE",
        "IDENTIFIER"
    };

    static constexpr const char * get_token_type_name(TOKEN_TYPE token) {
        return TokenMap[token];
    }

    static TOKEN_TYPE get_token_type_by_name(std::string raw_value) {
        static constexpr const char * TOKEN_IDENTIFIER_REGEX = "^\"[\\w\\./]+\"$";

        if (raw_value == "SELECT") return SELECT;
        if (raw_value == "FROM")   return FROM;
        if (raw_value == "WHERE")  return WHERE;
        if (raw_value == "NAME")   return NAME;
        if (raw_value == "SIZE")   return SIZE;
        if (raw_value == "," )     return COMMA;
        if (std::regex_match(raw_value, std::regex(TOKEN_IDENTIFIER_REGEX))) return IDENTIFIER;
        return UNKNOWN; 
    }

    class Token {

        private:
            TOKEN_TYPE type;
            std::string value;
        public:
            Token(
                TOKEN_TYPE type,
                std::string value
            ) {
                this->type = type;
                this->value = value;
            }
            TOKEN_TYPE get_type() {
                return this->type;
            }

            std::string get_value() {
                return this->value;
            }

            bool is_terminal() {
                // for now, just the identifier is terminal
                return this->type == IDENTIFIER;
            }        
    };

    class Lexical {

        private:
            std::string raw_input;
            uint32_t current_index;
            std::vector<Token> tokens;
        public:
            Lexical(std::string raw_input) {

                // Read the user input, and, split the raw string input, into an array of values splited by whitespace.
                static constexpr const char * const NON_WHITESPACES_REGEX = "(\\S)*(\\w+|,)(\\S)*";

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

                    #ifdef DEBUG_COMPILER
                            std::cout << "Found match: " <<  match_str << " Token type: " << get_token_type_name(token_type) << std::endl;
                    #endif
                }
            }

            Token next_token() {
                return tokens[current_index++];
            }

            bool has_next() {
                return tokens.size() > current_index;
            }

            void reset() {
                current_index = 0;
            }
    };

    class Grammar {
        private:
            std::vector<TOKEN_TYPE> expectations;
        private:
        public:
            Grammar(TOKEN_TYPE token){ 
                expectations.push_back(token);
            };
            Grammar(TOKEN_TYPE token, TOKEN_TYPE token2){ 
                expectations.push_back(token);
                expectations.push_back(token2);
            };
            
            bool match(TOKEN_TYPE type) {
                return std::find((expectations).begin(), (expectations).end(), type) != (expectations).end();
            };
    };

    class Syntax {

        private:
            std::vector<std::string> errors_list;
            std::map<TOKEN_TYPE, Grammar> *grammars; 
        public:
            Syntax() {
                grammars = new std::map<TOKEN_TYPE, Grammar>;
                grammars->insert(std::pair<TOKEN_TYPE, Grammar>(SELECT, Grammar(NAME, SIZE)));
                grammars->insert(std::pair<TOKEN_TYPE, Grammar>(COMMA, Grammar(NAME, SIZE)));
                grammars->insert(std::pair<TOKEN_TYPE, Grammar>(NAME, Grammar(COMMA, FROM)));
                grammars->insert(std::pair<TOKEN_TYPE, Grammar>(SIZE, Grammar(COMMA, FROM)));
                grammars->insert(std::pair<TOKEN_TYPE, Grammar>(FROM, Grammar(IDENTIFIER)));
            }
            ~Syntax() {
                delete grammars;
            }
            std::vector<std::string> get_errors() {
                return this->errors_list;
            }
            bool analyse(Lexical &lexical) {
                
                auto previous = lexical.next_token();

                while (lexical.has_next()) {
                    auto token = lexical.next_token();
                    if (grammars->find(previous.get_type())->second.match(token.get_type())) {
                        // OK, grammar is ok
                        previous = token;
                    } else {
                        // invalid grammar
                        std::ostringstream string_buffer;
                        string_buffer << "Error! Invalid Token!";  
                        string_buffer << "\tFrom the token: " << get_token_type_name(previous.get_type()) << ".";
                        errors_list.push_back(string_buffer.str());
                        break;
                    }
                }
                return errors_list.size() == 0;                
            }
    };
};

Termsequel::Compiler::Compiler(std::string raw_input) {
    this->raw_input = raw_input;
}

void Termsequel::Compiler::execute() {

    Lexical lexical(this->raw_input);
    Syntax syntax;
    if (!syntax.analyse(lexical)) {
        std::cerr << "Errors found!" <<std::endl;
        for(auto element : syntax.get_errors()) {
            std::cerr << element << std::endl;
        }
        return;
    } 

    lexical.reset();

    // Terminal token
    auto token = lexical.next_token();

    while (lexical.has_next()) token = lexical.next_token();

    auto vector = System::get_information(token.get_value());
    if (vector) {
        for(auto element: *vector) {
            std::cout << "Filename: " << element->get_name() << " Size: " << element->get_size() << std::endl;
            delete element;
        }
    }    
    delete vector;
}