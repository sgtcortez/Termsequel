#ifndef __TERMSEQUEL_LEXER__
#define __TERMSEQUEL_LEXER__

#include <cstdint>
#include <set>
#include <stack>
#include <string>

#include "token.hpp"

/**
 * This source responsability is to normalize the user input in a friendly
 * token list.
 */

namespace termsequel {
namespace lexer {
class Lexer {
  private:
    std::uint16_t             current_index;
    std::stack<std::uint16_t> previous_indexes;
    const std::string         input;

    token::Token eat ();

  public:
    Lexer ( const std::string input );
    ~Lexer () = default;

    bool         has_next () const;
    token::Token next ();
    token::Token peek ();
    token::Token previous ();
    void         puke ();
};
}; // namespace lexer

}; // namespace termsequel

#endif