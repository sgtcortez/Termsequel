#ifndef __TERMSEQUEL_LEXICAL_H__
#define __TERMSEQUEL_LEXICAL_H__

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "lexeme.hpp"

namespace Termsequel {

   class Lexical final {
      private:
         std::vector<std::shared_ptr<Lexeme>> lexemes;
         void convert_input(const std::string raw_input);
      public:
         Lexical(const std::string raw_input);
         std::shared_ptr<Lexeme> operator[](std::uint32_t index) const;
         bool has_next(std::uint32_t index) const;
   };

};

#endif