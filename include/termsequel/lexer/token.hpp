#ifndef __TERMSEQUEL_TOKEN__
#define __TERMSEQUEL_TOKEN__

#include <cstdint>
#include <ostream>
#include <string>

namespace termsequel {
namespace token {
enum class Type : std::uint8_t
{
    INVALID               = 0,
    SELECT                = 1,
    FROM                  = 2,
    WHERE                 = 3,
    AND                   = 4,
    OR                    = 5,
    COMMA                 = 6,
    MULTIPLICATION        = 7,
    STRING                = 8,
    IDENTIFIER            = 9,
    NUMBER                = 10,
    PLUS                  = 11,
    MINUS                 = 12,
    DIVIDE                = 13,
    EQUAL                 = 14,
    NOT_EQUAL             = 15,
    LEFT_PARENTHESIS      = 16,
    RIGHT_PARENTHESIS     = 17,
    AS                    = 18,
    COUNT                 = 19,
    GROUP                 = 20,
    BY                    = 21,
    ORDER                 = 22,
    DESC                  = 23,
    ASC                   = 24,
    LESS_THAN             = 25,
    LESS_THAN_OR_EQUAL    = 26,
    GREATER_THAN          = 27,
    GREATER_THAN_OR_EQUAL = 28,
    UPPER                 = 29,
    LOWER                 = 30,
    LIKE                  = 31,
    BOOLEAN               = 32,
    UNARY_MINUS           = 33,
    NOT                   = 34,
};

enum class Class : std::uint8_t
{
    /**
     * SELECT, FROM, WHERE ...
     */
    KEYWORD = 0,

    /**
     * +, -, / ...
     */
    ARITHMETIC_OPERATOR = 1,

    /**
     * Special are some strings that their meaning depends on the context
     * For example, STAR(*), may mean all columns available as well a multiplier
     */
    SPECIAL = 2,

    /**
     * AND, OR
     */
    LOGICAL_OPERATOR = 3,

    /**
     * >, =, >=, <=, <, !=, <> ...
     */
    RELATIONAL_OPERATOR = 4,

    /**
     * Entries.
     * Numbers, strings, identifiers ...
     */
    ENTRY = 5,

    /**
     A function that should be applied.
     COUNT, UPPER, LOWER, LIKE ...
    */
    FUNCTION = 6,
};

class Token final {
  private:
    Type        type;
    Class       clazz;
    std::string value;

  public:
    ~Token () = default;
    Token ( const Type type, const Class clazz, const std::string value );

    bool              operator== ( const Token &other ) const;
    Type              get_type () const;
    Class             get_class () const;
    const std::string get_value () const;

    friend std::ostream &operator<< ( std::ostream &stream, const Token &token )
    {
        const char *clazz_values[] = { "KEYWORD", "ARITHMETIC", "SPECIAL", "LOGICAL", "RELATIONAL", "ENTRY", "FUNCTION" };
        stream << "Token(" << clazz_values[ static_cast<std::uint8_t> ( token.clazz ) ] << ":'" << token.value << "')";
        return stream;
    }
};

}; // namespace token

}; // namespace termsequel

#endif