#ifndef __TERMSEQUEL_LEXEME_H__
#define __TERMSEQUEL_LEXEME_H__

#include <cstdint>
#include <string>
#include <memory> // for shared_ptr

namespace Termsequel {

   enum class LexemeKey {
      OPERATION,   // Operation to be performed. For now, just SELECT
      COLUMN,      // Columns. NAME, SIZE etc ...
      IDENTIFIER,  // The identifier. Can be a directory or file name.
      WHERE,       // Filter
      COMMA,       // To include more columns to retrieve
      FROM,        // From where to search
      COMPARASION, // Compare something. =, CONTAINS, >= etc ...
      LOGICAL,     // Logical operations. Like: AND and OR.
   };

   class LexemeType final {
      private:
         const LexemeKey lexeme_key;
         const std::string name;
         LexemeType (
            const LexemeKey lexeme_key,
            std::string name
         );
      public:
         LexemeKey get_key() const;
         const std::string get_name() const;
         bool operator==(const LexemeType & right) const;

         // Static instances, trying to represent the Java Enums ...
         static const LexemeType SELECT;
         static const LexemeType STAR;
         static const LexemeType NAME;
         static const LexemeType SIZE;
         static const LexemeType OWNER;
         static const LexemeType LEVEL;
         static const LexemeType FILE_TYPE;
         static const LexemeType OWNER_PERMISSIONS;
#ifdef __linux__
         static const LexemeType GROUP_PERMISSIONS;
         static const LexemeType OTHERS_PERMISSIONS;
#endif
         static const LexemeType LAST_MODIFICATION;
         static const LexemeType CREATION_DATE;
         static const LexemeType RELATIVE_PATH;
         static const LexemeType ABSOLUTE_PATH;
         static const LexemeType COMMA;
         static const LexemeType FROM;
         static const LexemeType IDENTIFIER;
         static const LexemeType WHERE;
         static const LexemeType EQUAL;
         static const LexemeType NOT_EQUAL;
         static const LexemeType STARTS_WITH;
         static const LexemeType NOT_STARTS_WITH;
         static const LexemeType ENDS_WITH;
         static const LexemeType NOT_ENDS_WITH;
         static const LexemeType CONTAINS;
         static const LexemeType NOT_CONTAINS;
         static const LexemeType BIGGER;
         static const LexemeType LESS;
         static const LexemeType BIGGER_OR_EQUAL;
         static const LexemeType LESS_OR_EQUAL;
         static const LexemeType AND;
         static const LexemeType OR;
   };

   class Lexeme final {
      private:
         // those are static instances, so, should not be deleted
         const LexemeType & lexeme_type;

         // heap allocated object, should be deleted
         const std::shared_ptr<std::string> value;
      public:
         Lexeme (const LexemeType &lexeme_type);
         Lexeme (
            const LexemeType &lexeme_type,
            const std::string value
         );
         const LexemeType  & get_type () const;
         const std::shared_ptr<std::string>  get_value() const;

   };
};


#endif