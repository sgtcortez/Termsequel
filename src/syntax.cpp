#include <cstdint>
#include <iostream>
#include <iterator>

#include "syntax.hpp"


bool matches(
   const Termsequel::LexemeKey &left,
   const Termsequel::LexemeKey &right
) {
   switch (left) {

      case Termsequel::LexemeKey::OPERATION:
      case Termsequel::LexemeKey::WHERE:
      case Termsequel::LexemeKey::COMMA:
      case Termsequel::LexemeKey::LOGICAL:
         return (right == Termsequel::LexemeKey::COLUMN);

      case Termsequel::LexemeKey::FROM:
      case Termsequel::LexemeKey::COMPARASION:
         return (right == Termsequel::LexemeKey::IDENTIFIER);

      case Termsequel::LexemeKey::COLUMN:
         return (right == Termsequel::LexemeKey::COMMA || right == Termsequel::LexemeKey::FROM || right == Termsequel::LexemeKey::COMPARASION);

      case Termsequel::LexemeKey::IDENTIFIER:
         return (right == Termsequel::LexemeKey::WHERE || right == Termsequel::LexemeKey::LOGICAL);
      default:
         // should not enter here
         return false;
   }

}

bool Termsequel::Syntax::analyse(Termsequel::Lexical &lexical) const {

   bool found_where = false;
   std::uint32_t index = 0;

   while(true) {
      const auto left = lexical[index++];
      // reached the end
      if (!(lexical.has_next(index))) break;

      const auto right = lexical[index];

      if( !(matches(left->get_type().get_key(), right->get_type().get_key()))) {
         // error
         std::cerr << "Syntax error!" << std::endl;
         std::cerr << "Near tokens: \"" << left->get_type().get_name() << "\" and \"" << right->get_type().get_name() << "\"." << std::endl;
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