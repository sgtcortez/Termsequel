#ifndef __TERMSEQUEL_SYNTAX_H__
#define __TERMSEQUEL_SYNTAX_H__

#include "lexical.hpp"
#include <vector>

namespace Termsequel {

    class Syntax {

        private:
            std::vector<std::string> errors_list;
        public:
            Syntax();
            std::vector<std::string> get_errors();
            bool analyse(Lexical &lexical);

    };

}


#endif /* __TERMSEQUEL_SYNTAX_H__ */