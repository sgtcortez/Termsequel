#ifndef __TERMSEQUEL_PARSER__
#define __TERMSEQUEL_PARSER__

#include "termsequel/lexer/lexer.hpp"
#include "termsequel/parser/node.hpp"

#include <memory>

/**
 * This source responsability is to parse the lexemes that were generated
 * by the lexer into an AbstractSyntaxTree as well an initial validation of the
 * syntax of the user input.
 */

namespace termsequel {
namespace parser {
/**
 * Parses the list of tokens into an Abstract Syntax Tree
 */
class Parser final {
  public:
    std::unique_ptr<StatementNode> parse ( lexer::Lexer &lexer ) const;
};
}; // namespace parser
}; // namespace termsequel
#endif
