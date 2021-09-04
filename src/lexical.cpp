#include "lexical.hpp"
#include <cstdint>
#include <memory>

Termsequel::Lexical::Lexical(const std::string raw_input) {
   convert_input(raw_input);
};

std::shared_ptr<Termsequel::Lexeme>  parse_lexeme(const std::string string) {
   if (      string.compare("SELECT")               == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::SELECT);
   else if ( string.compare("*")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::STAR);
   else if ( string.compare("NAME")                 == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::NAME);
   else if ( string.compare("SIZE")                 == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::SIZE);
   else if ( string.compare("OWNER")                == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::OWNER);
   else if ( string.compare("LEVEL")                == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::LEVEL);
   else if ( string.compare("FILE_TYPE")            == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::FILE_TYPE);
   else if ( string.compare("OWNER_PERMISSIONS")    == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::OWNER_PERMISSIONS);
#ifdef __linux__
   else if ( string.compare("GROUP_PERMISSIONS")    == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::GROUP_PERMISSIONS);
   else if ( string.compare("OTHERS_PERMISSIONS")   == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::OTHERS_PERMISSIONS);
#endif
   else if ( string.compare("LAST_MODIFICATION")    == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::LAST_MODIFICATION);
   else if ( string.compare("CREATION_DATE")        == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::CREATION_DATE);
   else if ( string.compare("RELATIVE_PATH")        == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::RELATIVE_PATH);
   else if ( string.compare("ABSOLUTE_PATH")        == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::ABSOLUTE_PATH);
   else if ( string.compare(",")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::COMMA);
   else if ( string.compare("FROM")                 == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::FROM);
   else if ( string.compare("WHERE")                == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::WHERE);
   else if ( string.compare("=")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::EQUAL);
   else if ( string.compare("STARTS_WITH")          == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::STARTS_WITH);
   else if ( string.compare("NOT_STARTS_WITH")      == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::NOT_STARTS_WITH);
   else if ( string.compare("ENDS_WITH")            == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::ENDS_WITH);
   else if ( string.compare("NOT_ENDS_WITH")        == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::NOT_ENDS_WITH);
   else if ( string.compare("CONTAINS")             == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::CONTAINS);
   else if ( string.compare("NOT_CONTAINS")         == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::NOT_CONTAINS);
   else if ( string.compare(">")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::BIGGER);
   else if ( string.compare("<")                    == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::LESS);
   else if ( string.compare(">=")                   == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::BIGGER_OR_EQUAL);
   else if ( string.compare("<=")                   == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::LESS_OR_EQUAL);
   else if ( string.compare("AND")                  == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::AND);
   else if ( string.compare("OR")                   == 0 ) return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::OR);
   return std::make_shared<Termsequel::Lexeme>(Termsequel::LexemeType::IDENTIFIER, string);
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

