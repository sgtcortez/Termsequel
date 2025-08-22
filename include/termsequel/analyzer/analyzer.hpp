#ifndef __TERMSEQUEL_ANALYZER__
#define __TERMSEQUEL_ANALYZER__

#include "termsequel/parser/node.hpp"
#include "termsequel/parser/parser.hpp"

/**
 * This source infers some additional data to the AbstractSyntaxTree as well
 * does some validation on the semantic.
 */

namespace termsequel {
namespace analyzer {
class Analyzer {
  public:
    void analyze ( parser::StatementNode *abstract_syntax_tree ) const;
};
}; // namespace analyzer
}; // namespace termsequel

#endif