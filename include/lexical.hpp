#ifndef __TERMSEQUEL_LEXICAL_H__
#define __TERMSEQUEL_LEXICAL_H__

#include <cstdint>
#include <string>
#include <vector>

#include "token.hpp"

namespace Termsequel {

    class Lexical {

        private:
            std::string raw_input;
            uint32_t current_index;
            std::vector<Token> tokens;
        public:
            Lexical(std::string raw_input);
            Token next_token();
            bool has_next();
    };

};

#endif