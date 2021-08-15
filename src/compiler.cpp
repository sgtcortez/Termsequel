#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <map>
#include <ostream>
#include <sstream>
#include <streambuf>
#include <string>
#include <utility>
#include <iostream>
#include <vector>
#include <iostream>

#include "compiler.hpp"
#include "system.hpp"

#ifdef DEBUG
    #define DEBUG_COMPILER 1
#endif

#ifdef DEBUG_COMPILER
    #include <iostream>
#endif

namespace Termsequel {

   enum TokenType {
      TYPE_COMMAND, // A SQL command, like SELECT
      TYPE_COLUMN, // An available column
      TYPE_IDENTIFIER, // An identifier
      TYPE_WHERE, // A restriction
      TYPE_COMMA, // A comma
      TYPE_FROM, // A target
      TYPE_END, // Represents the end of the input
      TYPE_COMPARASION, // Represents a comparasion
   };

   static constexpr const char * const TOKEN_MAP[] = {
      "COMMAND",
      "COLUMN",
      "IDENTIFIER",
      "WHERE",
      "COMMA",
      "FROM",
      "END",
      "COMPARASION"
   };

   static constexpr char WHITE_SPACE = ' ';
   static constexpr char COMMA       = ',';

   // Tries to simulate java enums ...
   class Token final {
      private:
         TokenType type;
         Token(TokenType type) {
            this->type = type;
         };
      public:

         // These are static instances
         static const Token SELECT;
         static const Token NAME;
         static const Token SIZE;
         static const Token FROM;
         static const Token IDENTIFIER;
         static const Token WHERE;
         static const Token COMMA;
         static const Token END;
         static const Token EQUAL;

         bool is_end() const {
            return type == TokenType::TYPE_END;
         }

         bool matches(Token next) const {
            switch (type) {
               case TokenType::TYPE_COMMAND:
                  return ( next.type == TokenType::TYPE_COLUMN );
               case TokenType::TYPE_COLUMN:
                  return ( next.type == TokenType::TYPE_COMMA || next.type == TokenType::TYPE_FROM || next.type == TokenType::TYPE_COMPARASION );
               case TokenType::TYPE_COMMA:
                  return ( next.type == TokenType::TYPE_COLUMN );
               case TokenType::TYPE_FROM:
                  return ( next.type == TokenType::TYPE_IDENTIFIER);
               case TokenType::TYPE_WHERE:
                  return ( next.type == TokenType::TYPE_COLUMN );
               case TokenType::TYPE_COMPARASION:
                  return ( next.type == TokenType::TYPE_IDENTIFIER );
               case TokenType::TYPE_IDENTIFIER:
                  return ( next.type == TokenType::TYPE_WHERE || next.type == TokenType::TYPE_END );
               default:
                  // Must not enter here, otherwise, there is a problem with the compiler
                  return false;
            };
         };

         bool operator==(const Token &right) const {
            const std::intptr_t left_address  = (std::intptr_t)this;
            const std::intptr_t right_address = (std::intptr_t)&right;
            return left_address == right_address;
         };

         const char * name() const {
            return TOKEN_MAP[type];
         }


   };

   // initializes the static instances.
   // There are no need to free this ...
   Token const Token::SELECT     = (TokenType::TYPE_COMMAND);
   Token const Token::NAME       = (TokenType::TYPE_COLUMN);
   Token const Token::SIZE       = (TokenType::TYPE_COLUMN);
   Token const Token::FROM       = (TokenType::TYPE_FROM);
   Token const Token::IDENTIFIER = (TokenType::TYPE_IDENTIFIER);
   Token const Token::WHERE      = (TokenType::TYPE_WHERE);
   Token const Token::COMMA      = (TokenType::TYPE_COMMA);
   Token const Token::END        = (TokenType::TYPE_END);
   Token const Token::EQUAL      = (TokenType::TYPE_COMPARASION);

   struct Lexeme {

      const Token *token;
      std::string *value;

      Lexeme (
         const Token &token
      ) {
         this->token = &token;
         value = nullptr;
      }

      Lexeme (
         const Token &token,
         std::string identifier
      ) {
         this->token = &token;
         this->value = new std::string(identifier);
      }

      ~Lexeme () {
         delete value;
      }

      bool is_terminal() const {
         return token->is_end();
      };

      // TODO: Does not work without friend, and, I do not know why
      friend std::ostream & operator << (std::ostream &output_stream, const Lexeme &lexeme) {
         output_stream << " Token: " << lexeme.token->name();
         if (lexeme.value) output_stream << " Value: " << *(lexeme.value);
         return output_stream;
      }

   };


   class Lexical {
      private:
         std::string raw_input;
         std::uint32_t current_index;

         Lexeme * parse_lexeme (const std::string string) {
            if (string.compare("SELECT") == 0 ) {
               return new Lexeme( Token::SELECT);
            } else if ( string.compare("NAME") == 0 ) {
               return new Lexeme( Token::NAME);
            } else if ( string.compare("SIZE") == 0 ) {
               return new Lexeme ( Token::SIZE );
            } else if ( string.compare("FROM") == 0 ) {
               return new Lexeme ( Token::FROM );
            } else if ( string.compare("WHERE") == 0 ) {
               return new Lexeme (Token::WHERE);
            } else if ( string.compare("=") == 0 ) {
               return new Lexeme (Token::EQUAL);
            } else if ( string.compare(",") == 0 ) {
               return new Lexeme ( Token::COMMA );
            } else {
               // anything else is an identifier
               return new Lexeme( Token::IDENTIFIER, string);
            }
         }

      public:
         Lexical(std::string raw_input) {
               this->raw_input = raw_input;
               current_index = 0;
         };

         Lexeme * next() {

            while (true) {
               if ( current_index >= raw_input.size() || raw_input[current_index] != WHITE_SPACE ) break;
               current_index++;
            }
            // No more things to parse, because, we already reached the end of the input
            if ( current_index >= raw_input.size() ) return new Lexeme (Token::END);
            auto tmp_index = raw_input.find(WHITE_SPACE, current_index);
            if ( tmp_index == std::string::npos ) {
               // Reached the end
               tmp_index = raw_input.size();
            }
            auto token_string = raw_input.substr(current_index, tmp_index - current_index);
            const auto comma_index = token_string.find(COMMA);
            if ( comma_index != std::string::npos ) {
               // there are commas appended with another value

               if (token_string[0] == COMMA) {
                  // The first character is a comma, so, parse the comma
                  // and points to the next character
                  current_index++;
                  return parse_lexeme(",");
               }
               token_string = token_string.substr(0, comma_index) ;
               // Now, tmp_index points to the first comma in the substring
               tmp_index = current_index + comma_index;
            }

            // updates the index
            current_index = tmp_index;

            // returns a new lexeme
            return parse_lexeme(token_string);
         };
    };

};

Termsequel::Compiler::Compiler(std::string raw_input) {
    this->raw_input = raw_input;
}

void Termsequel::Compiler::execute() {

   Lexical lex(this->raw_input);
   std::vector<Lexeme*> lexemes;

   bool has_errors = false;

   while (true) {
      const auto lexeme = lex.next();
      #ifdef DEBUG_COMPILER
         std::cout << *lexeme << std::endl;
      #endif
      lexemes.push_back(lexeme);
      if ((*lexeme).token->is_end()) break;
   }

   for (unsigned long index = 1L; index < lexemes.size(); index ++) {
      const auto previous = lexemes[index - 1];
      const auto current = lexemes[index];
      if (!(previous->token->matches(*(current->token)))) {
         std::cerr << "Invalid SYNTAX!" << std::endl;
         std::cerr << "Near Tokens:" << *previous << " and " << *current << std::endl;
         has_errors = true;
         break;
      }
   }

   if (!(has_errors)) {

      auto system_command = Command();

      for (const auto lexeme: lexemes) {

         const auto token = (lexeme)->token;
         if (Token::SELECT == *(token)) {
            system_command.command = COMMAND_TYPE::LIST;
         } else if ( Token::NAME == *(token) ) {
            system_command.columns.push_back(COLUMN_TYPE::FILENAME);
         } else if ( Token::SIZE == *(token) ) {
            system_command.columns.push_back((COLUMN_TYPE::FILESIZE));
         } else if ( Token::IDENTIFIER == *(token) ) {
            system_command.target = *(lexeme->value);
         }
      }

      const auto result_rows = System::execute(&system_command);
      for (const auto row : *(result_rows)) {
         std::cout << *(row) << std::endl;
         delete row;
      }
      delete result_rows;
   }



   // delete the lexemes ...
   for(auto lexeme : lexemes) delete lexeme;



}