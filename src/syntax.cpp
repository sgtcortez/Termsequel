#include <cstdint>
#include <iostream>
#include <iterator>

#include "lexeme.hpp"
#include "syntax.hpp"

using lexeme_key = Termsequel::LexemeKey;

bool matches(const lexeme_key &left, const lexeme_key &right) {
  switch (left) {

  case lexeme_key::OPERATION:
  case lexeme_key::WHERE:
  case lexeme_key::COMMA:
  case lexeme_key::LOGICAL:
    return (right == lexeme_key::COLUMN);

  case lexeme_key::FROM:
  case lexeme_key::COMPARASION:
    return (right == lexeme_key::IDENTIFIER);

  case lexeme_key::COLUMN:
    return (right == lexeme_key::COMMA || right == lexeme_key::FROM ||
            right == lexeme_key::COMPARASION);

  case lexeme_key::IDENTIFIER:
    return (right == lexeme_key::WHERE || right == lexeme_key::LOGICAL);
  default:
    // should not enter here
    return false;
  }
}

bool Termsequel::Syntax::analyse(Termsequel::Lexical &lexical) const {

  bool found_where = false;
  std::uint32_t index = 0;

  while (true) {
    const auto left = lexical[index++];
    // reached the end
    if (!(lexical.has_next(index)))
      break;

    const auto right = lexical[index];

    if (!(matches(left->get_type().get_key(), right->get_type().get_key()))) {
      // error
      std::cerr << "Syntax error!" << std::endl;
      std::cerr << "Near tokens: \"" << left->get_type().get_name()
                << "\" and \"" << right->get_type().get_name() << "\"."
                << std::endl;
      return false;
    }
    if (Termsequel::LexemeType::WHERE == right->get_type()) {
      if (found_where) {
        // error
        std::cerr << "Syntax Error! Can not have two where!" << std::endl;
        return false;
      } else {
        found_where = true;
      }
    }
  }
  return true;
}