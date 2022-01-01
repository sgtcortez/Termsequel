#include "lexical.hpp"
#include "lexeme.hpp"
#include <cstdint>
#include <memory>

using lexeme_type = Termsequel::LexemeType;

Termsequel::Lexical::Lexical(const std::string raw_input) {
   convert_input(raw_input);
};

std::shared_ptr<Termsequel::Lexeme>  parse_lexeme(const std::string string) {
   if (      string.compare("SELECT")               == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::SELECT);
   else if ( string.compare("*")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::STAR);
   else if ( string.compare("NAME")                 == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::NAME);
   else if ( string.compare("SIZE")                 == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::SIZE);
   else if ( string.compare("OWNER")                == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::OWNER);
#ifdef __linux__
   else if ( string.compare("GROUP")                == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::GROUP);
#endif
   else if ( string.compare("LEVEL")                == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::LEVEL);
   else if ( string.compare("FILE_TYPE")            == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::FILE_TYPE);
   else if ( string.compare("OWNER_PERMISSIONS")    == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::OWNER_PERMISSIONS);
#ifdef __linux__
   else if ( string.compare("GROUP_PERMISSIONS")    == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::GROUP_PERMISSIONS);
   else if ( string.compare("OTHERS_PERMISSIONS")   == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::OTHERS_PERMISSIONS);
#endif
   else if ( string.compare("LAST_MODIFICATION")    == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::LAST_MODIFICATION);
   else if ( string.compare("CREATION_DATE")        == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::CREATION_DATE);
   else if ( string.compare("RELATIVE_PATH")        == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::RELATIVE_PATH);
   else if ( string.compare("ABSOLUTE_PATH")        == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::ABSOLUTE_PATH);
   else if ( string.compare(",")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::COMMA);
   else if ( string.compare("FROM")                 == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::FROM);
   else if ( string.compare("WHERE")                == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::WHERE);
   else if ( string.compare("=")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::EQUAL);
   else if ( string.compare("!=")                   == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::NOT_EQUAL);
   else if ( string.compare("STARTS_WITH")          == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::STARTS_WITH);
   else if ( string.compare("NOT_STARTS_WITH")      == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::NOT_STARTS_WITH);
   else if ( string.compare("ENDS_WITH")            == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::ENDS_WITH);
   else if ( string.compare("NOT_ENDS_WITH")        == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::NOT_ENDS_WITH);
   else if ( string.compare("CONTAINS")             == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::CONTAINS);
   else if ( string.compare("NOT_CONTAINS")         == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::NOT_CONTAINS);
   else if ( string.compare(">")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::BIGGER);
   else if ( string.compare("<")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::LESS);
   else if ( string.compare(">=")                   == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::BIGGER_OR_EQUAL);
   else if ( string.compare("<=")                   == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::LESS_OR_EQUAL);
   else if ( string.compare("AND")                  == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::AND);
   else if ( string.compare("OR")                   == 0 ) return std::make_shared<Termsequel::Lexeme>(lexeme_type::OR);
   return std::make_shared<Termsequel::Lexeme>(lexeme_type::IDENTIFIER, string);
}

std::shared_ptr<Termsequel::Lexeme>  Termsequel::Lexical::operator[](std::uint32_t index) const {
   return lexemes[index];
}

bool Termsequel::Lexical::has_next(std::uint32_t index) const {
   return lexemes.size() > index;
}

void Termsequel::Lexical::convert_input(const std::string raw_input) {
   std::uint64_t current_index = 0;
   while (raw_input.size() > current_index) {
      while (raw_input.size() > current_index && raw_input[current_index] == ' ') current_index++;

      if (current_index >= raw_input.size()) {
         // reached the end
         // does nothing
         continue;
      }

      // Searches for whitespace
      auto index = raw_input.find(' ', current_index);
      if (index == std::string::npos) {
         // There is no more whitespace characters
         index = raw_input.size();
      }

      // The token to use
      auto token = raw_input.substr(current_index, index - current_index);
      const auto comma_index = token.find(',');
      if (comma_index != std::string::npos) {
         // there is a comma in the current token
         if (token[0] == ',') {
            // the first character is a comma, we have here something like: ,SIZE
            // we need to threat the comma as a seperate Lexeme.
            current_index++;
            lexemes.push_back(parse_lexeme(","));
            continue;
         }
         // the comma is between to strings, or at the end, something like: NAME,SIZE or NAME,
         token = token.substr(0, comma_index);
         index = current_index + comma_index;
      }
      current_index = index;
      lexemes.push_back(parse_lexeme(token));
   }
}

