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
         std::string string
      ) {
         this->token = token;
         value = new std::string(string);
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

         Lexeme * parse_lexeme (const std::string string) {
            if (string.compare("SELECT") == 0 ) {
               return new Lexeme( Token::SELECT, string );
            } else if ( string.compare("NAME") == 0 ) {
               return new Lexeme( Token::NAME, string);
            } else if ( string.compare("SIZE") == 0 ) {
               return new Lexeme ( Token::SIZE, string );
            } else if ( string.compare("FROM") == 0 ) {
               return new Lexeme ( Token::FROM, string );
            } else if ( string.compare("WHERE") == 0 ) {
               return new Lexeme (Token::WHERE, string);
            } else if ( string.compare("=") == 0 ) {
               return new Lexeme (Token::EQUAL, string);
            } else if ( string.compare(",") == 0 ) {
               return new Lexeme ( Token::COMMA, string );
            } else {
               // anything else is an identifier
               return new Lexeme(Token::IDENTIFIER, string);
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
            // No more things to parse, because, we already reached the end of the input
            if ( current_index >= raw_input.size() ) return new Lexeme (Token::END,  "End of input");
            auto tmp_index = raw_input.find(' ', current_index);
            if ( tmp_index == std::string::npos ) {
               // Reached the end
               tmp_index = raw_input.size();
            }
            auto token_string = raw_input.substr(current_index, tmp_index - current_index);
            const auto comma_index = token_string.find(",");
            if ( comma_index != std::string::npos ) {
               // there are commas appended with another value

               if (token_string[0] == ',') {
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

   bool has_errors = false;

   while (true) {
      const auto lexeme = lex.next();
      #ifdef DEBUG_COMPILER
         std::cout << *lexeme << std::endl;
      #endif
      lexemes.push_back(lexeme);
      if ((*lexeme).token == END) break;
   }

   for (unsigned long index = 1L; index < lexemes.size(); index ++) {
      const auto previous = lexemes[index - 1];
      const auto current = lexemes[index];
      if (!(match(previous->token, current->token))) {
         std::cerr << "Invalid SYNTAX!" << std::endl;
         std::cerr << "Near Tokens:" << *previous << " and " << *current << std::endl;
         has_errors = true;
         break;
      }
   }
   if (!(has_errors)) {

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

   // delete the lexemes ...
   for(auto lexeme : lexemes) delete lexeme;



}