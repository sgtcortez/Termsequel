#ifndef __TERMSEQUEL_TOKEN_H__
#define __TERMSEQUEL_TOKEN_H__

#include <cstdint>
#include <string>

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

    extern const char * const TokenMap[];
    extern const char * get_token_type_name(TOKEN_TYPE token);
    extern const char * TOKEN_IDENTIFIER_REGEX;
    extern TOKEN_TYPE get_token_type_by_name(std::string raw_value);

    class Token {

        private:
            TOKEN_TYPE type;
            std::string value;
        public:
            Token(
                TOKEN_TYPE type,
                std::string value
            );
            TOKEN_TYPE get_type();
            std::string get_value();
            bool is_terminal();
    
    };


}

#endif