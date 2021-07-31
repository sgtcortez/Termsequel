#ifndef __TERMSEQUEL_TOKEN_H__
#define __TERMSEQUEL_TOKEN_H__

#include <cstdint>
#include <string>

namespace Termsequel {

    enum TOKEN_TYPE {
        SELECT = 0,     // SQL SELECT KEYWORD
        FROM = 1,       // SQL FROM KEYWORD
        WHERE = 2,      // SQL WHERE KEYWORD
        IDENTIFIER = 3, // An identifer. A directory name, file name for example
        UNKNOWN = 4,    // Invalid. Unreconized token
        COMMA = 5,      // Comma to separate columns 
        NAME = 6        // NAME column. 
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
    
    };


}

#endif