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



namespace Termsequel {

   enum TOKEN_TYPE {
      TYPE_COMMAND, // A SQL command, like SELECT
      TYPE_COLUMN, // An available column
      TYPE_IDENTIFIER, // An identifier
      TYPE_WHERE, // A restriction
      TYPE_COMMA, // A comma
      TYPE_FROM, // A target
      TYPE_EQUAL, // equal
      TYPE_END, // Represents the end of the input
   };

   static constexpr const char * const TOKEN_MAP[] = {
      "COMMAND",
      "COLUMN",
      "IDENTIFIER",
      "WHERE",
      "COMMA",
      "FROM",
      "EQUAL",
      "END"
   };

   enum Token {
      SELECT = TYPE_COMMAND,
      FROM = TYPE_FROM,
      WHERE = TYPE_WHERE,
      NAME = TYPE_COLUMN,
      SIZE = TYPE_COLUMN,
      IDENTIFIER = TYPE_IDENTIFIER,
      EQUAL = TYPE_EQUAL,
      END = TYPE_END,
      COMMA = TYPE_COMMA,
   };

   struct Lexeme {

      enum Token token;
      std::string *value;

      Lexeme (
         enum Token token,
         const char *string,
         std::uint32_t size
      ) {
         this->token = token;
         value = new std::string(string, size);
      }

      ~Lexeme() {
         delete value;
      }

      bool is_terminal() {
         return token == Token::END;
      };

      // TODO: Does not work without friend, and, I do not know why
      friend std::ostream & operator << (std::ostream &output_stream, const Lexeme &lexeme) {
         output_stream << " Token: " << TOKEN_MAP[lexeme.token];
         output_stream << " Value: " << *(lexeme.value);
         return output_stream;
      }

   };

   class Lexical {
      private:
         std::string raw_input;
         std::uint32_t current_index;

         Lexeme * parse_lexeme (
               const char * c_string,  /* The cstring to compare*/
               std::uint32_t size      /* Number of characters to compare*/
         ) {
            // TODO: Refactor to use string::compare method
            // https://www.cplusplus.com/reference/string/string/compare/
            if (        std::strncmp(c_string, "SELECT", size ) == 0 ) {
               return new Lexeme ( SELECT, c_string, size );
            } else if ( std::strncmp(c_string, "NAME",   size ) == 0 ) {
               return new Lexeme ( Token::NAME,   c_string, size );
            } else if ( std::strncmp(c_string, "SIZE",   size ) == 0 ) {
               return new Lexeme ( Token::SIZE,   c_string, size );
            } else if ( std::strncmp(c_string, "FROM",   size ) == 0 ) {
               return new Lexeme ( Token::FROM,   c_string, size );
            } else if ( std::strncmp(c_string, "WHERE",  size ) == 0 ) {
               return new Lexeme ( Token::WHERE,  c_string, size );
            } else if ( std::strncmp(c_string, "=",      size ) == 0 ) {
               return new Lexeme ( Token::EQUAL,  c_string, size );
            } else if ( std::strncmp(c_string, ",",      size)  == 0 ) {
               return new Lexeme ( Token::COMMA,  c_string, size );
            } else {
               // anything else, is an identifier
               return new Lexeme (Token::IDENTIFIER, c_string, size );
            }

         }

      public:
         Lexical(std::string raw_input) {
               this->raw_input = raw_input;
               current_index = 0;
         };

         Lexeme * next() {

            while (true) {
               if ( current_index >= raw_input.size() || raw_input[current_index] != ' ' ) break;
               current_index++;
            }
            // No moe things to parse, because, we already reached the end of the input
            if ( current_index >= raw_input.size() ) return new Lexeme (Token::END,  "End of input", strlen("End of input"));
            auto tmp_index = raw_input.find(' ', current_index);
            if ( tmp_index == std::string::npos ) {
               // Reached the end
               tmp_index = raw_input.size();
            }
            const auto token_string = raw_input.substr(current_index, tmp_index - current_index);
            const auto comma_index = token_string.find(",");
            if ( comma_index != std::string::npos ) {
               // there are commas appended with another value

               if (token_string[0] == ',') {
                  current_index++;
                  return parse_lexeme(token_string.c_str(), 1);
               }

               // Now, tmp_index points to the first comma in the substring
               tmp_index = current_index + comma_index;
            }

            // Points to the first character of the new token
            const char *tmp_string = raw_input.c_str()+current_index;

            // how many characters should be considered
            const auto difference = tmp_index - current_index;

            // updates the index
            current_index = tmp_index;

            // returns a new lexeme
            return parse_lexeme(tmp_string, difference);
         };
    };

    bool match(
        Token token,
        Token received
    ) {
      switch (token) {
         case SELECT:
               // SELECT NAME ...  OR SELECT SIZE ...
               return (received == NAME || received == SIZE);
         case NAME:
               // since NAME and SIZE point to the same value, we do not need to handle them
               // they both are columns
               // SELECT NAME, ... OR SELECT NAME FROM ... OR SELECT ... WHERE NAME = ...
               return (received == COMMA || received == FROM || received == EQUAL);
         case COMMA:
               // name and size point to the same value
               // SELECT ..., NAME OR SELECT ..., SIZE
               return (received == NAME || received == SIZE);
         case FROM:
               // SELECT ... FROM USER_DEFINED_VALUE
               return (received == IDENTIFIER);
         case WHERE:
               // SELECT ... FROM IDENTIFIER WHERE NAME OR SELECT ... FROM IDENTIFIER WHERE SIZE
               return (received == NAME || received == SIZE);
         case EQUAL:
               // SELECT ... WHERE NAME = ...
               return (received == IDENTIFIER);
         case IDENTIFIER:
               // since the where is optional, the expected value from identifier can be the where and the end
               // SELECT ... FROM USER_DEFINED_VALUE OR SELECT .. FROM USER_DEFINED_VALUE WHERE COLUMN = USER_DEFINED_VALUE
               return (received == WHERE || received == END);
         case END:
               // THE END OF THE INPUT STREAM
               return true;
      };
    };
};

Termsequel::Compiler::Compiler(std::string raw_input) {
    this->raw_input = raw_input;
}

void Termsequel::Compiler::execute() {

   Lexical lex(this->raw_input);
   std::vector<Lexeme*> lexemes;

   while (true) {
      const auto lexeme = lex.next();
      std::cout << *lexeme << std::endl;
      lexemes.push_back(lexeme);
      if ((*lexeme).token == END) break;
   }

   for (unsigned long index = 1L; index < lexemes.size(); index ++) {
      const auto previous = lexemes[index - 1];
      const auto current = lexemes[index];
      if (!(match(previous->token, current->token))) {
         std::cerr << "Invalid SYNTAX!" << std::endl;
         std::cerr << "Near Tokens:" << *previous << " and " << *current << std::endl;
         return;
      }
   }

    auto system_command = Command();

   // Converts to the operating system interface
   for (auto element : lexemes) {
      switch (element->token) {
         case SELECT:
               system_command.command = COMMAND_TYPE::LIST;
               break;
         case NAME:
               // name and size point to the same integer value
               if ( element->value->compare("NAME") == 0 ) {
                  // name
                  system_command.columns.push_back(COLUMN_TYPE::FILENAME);
               } else {
                  // size
                  system_command.columns.push_back(COLUMN_TYPE::FILESIZE);
               }
               break;
         case IDENTIFIER:
               // target file/directory
               system_command.target = *(element->value);
               break;
         case WHERE:
               break;
         case EQUAL:
               break;
         default:
               // does nothing
               break;

         // frees the memory
         delete element;
      }
   }

   auto result = System::execute(&system_command);
   for ( auto element : *result ) {
      std::cout << *element << std::endl;
      delete element;
   }
   delete result;

}