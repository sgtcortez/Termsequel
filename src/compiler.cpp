#include <algorithm>
#include <cstdint>
#include <cstring>
#include <map>
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
        IDENTIFIER, // An identifer. A directory name, file name for example
        TOKEN_END   // There are no more token.
    };

    static constexpr const char * const TokenMap[] = {
        "UNKNOWN",
        "COMMA", 
        "SELECT",
        "FROM",
        "WHERE",
        "NAME",
        "SIZE",
        "IDENTIFIER",
        "TOKEN_END"
    };

    static constexpr const char * get_token_type_name(TOKEN_TYPE token) {
        return TokenMap[token];
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
        public:
            Lexical(std::string raw_input) {
                this->raw_input = raw_input;
                current_index = 0;
            }

            Token next_token() {
                while(true) {
                    if ( current_index >= raw_input.size() || raw_input[current_index] != ' ' ) break;
                    current_index++;
                }
                if (current_index >= raw_input.size()) return Token(TOKEN_END, "");
                auto tmp_index = raw_input.find(' ', current_index);
                if (tmp_index == std::string::npos) {
                    // Reached the end of the input
                    std::string identifier = raw_input.substr(current_index);
                    current_index = raw_input.size();
                    return Token(IDENTIFIER, identifier);    
                } else {
                    // updates the current index
                    const auto substring = raw_input.substr(current_index, tmp_index - current_index);
                    if (substring[0] == ',') {
                        // comma is the first character
                        // something like
                        // SELECT NAME ,SIZE ...
                        current_index++;
                        return Token(COMMA, "");
                    } else {
                        const auto comma_index = substring.find(",");
                        if (comma_index != std::string::npos) {
                            // has comma, but, is not the first character
                            tmp_index = current_index + comma_index;
                        } 
                    }
                    const char *tmp_string = raw_input.c_str()+current_index;
                    auto difference = tmp_index - current_index;
                    current_index = tmp_index;
                    if ( strncmp(tmp_string, "SELECT", difference) == 0 ) {
                        return Token(SELECT, "");    
                    } else if ( strncmp(tmp_string, "NAME", difference) == 0 ) {
                        return Token(NAME, "");
                    } else if ( strncmp(tmp_string, "SIZE", difference) == 0 ) {
                        return Token(SIZE, "");
                    } else if ( strncmp(tmp_string, "FROM", difference)  == 0) {
                        return Token(FROM, "");
                    } else {
                        // unknown token
                        return Token(UNKNOWN, "");
                    }   
                }
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
                auto token = lexical.next_token();
                while (token.get_type() != TOKEN_END) {
                    if (grammars->find(previous.get_type())->second.match(token.get_type())) {
                        // OK, grammar is ok
                        previous = token;
                        token = lexical.next_token();
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

    // resets to the first token
    lexical.reset();

    // SELECT token
    auto token = lexical.next_token();

    // until reache the terminal token(which contains the file name)
    while (true) {
        token = lexical.next_token();

        // found the identifier token
        if (token.get_type() == IDENTIFIER) break;
    }

    auto vector = System::get_information(token.get_value());
    for(auto element: *vector) {
        std::cout << "Filename: " << element->get_name() << " Size: " << element->get_size() << std::endl;
        delete element;
    }
    delete vector;
}