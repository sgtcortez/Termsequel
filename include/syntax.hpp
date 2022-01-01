#ifndef __TERMSEQUEL_SYNTAX_H__
#define __TERMSEQUEL_SYNTAX_H__

#include "lexical.hpp"

namespace Termsequel {

class Syntax final {

public:
  bool analyse(Termsequel::Lexical &lexical) const;
};

}; // namespace Termsequel

#endif
