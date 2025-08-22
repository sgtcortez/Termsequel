#include "termsequel/lexer/lexer.hpp"
#include "termsequel/lexer/token.hpp"

#include <cstdint>
#include <regex>
#include <vector>

using namespace termsequel;
using namespace lexer;

using namespace std;

regex spliter_normalizer (
    "((\\d+\\.\\d+)|(\\d+))|(TRUE|FALSE|UPPER)|((\"[\\w\\.-\\/"
    "]*\"*)|(\"*\\w*\"))|(_*\\w+_*)|(<>)|(>=)|(<=)|(!=)|([\\*+-/"
    "=,\\(\\\\>\\<)])" );
regex number ( "^[(\\d+\\.\\d+)|(\\d+)]+$" );

/**
* This function just normalize the user input.
* To be more friendly for the parser

Ie:
    SELECT * FROM MYTABLE -> REMAINS THE SAME
    SELECT*FROM MYTABLE -> SELECT * FROM MYTABLE
    SELECT * FROM MYTABLE WHERE 1=1 -> SELECT * FROM MYTABLE WHERE 1 = 1
*/
string normalize ( const string input )
{
    auto   words_begin = std::sregex_iterator ( input.begin (), input.end (), spliter_normalizer );
    auto   words_end   = std::sregex_iterator ();
    string result;

    for ( std::sregex_iterator i = words_begin; i != words_end; ++i )
        {
            std::smatch match     = *i;
            std::string match_str = match.str ();
            result                = result + match_str + " ";
        }
    return result;
}

vector<token::Token> TOKENS = { token::Token ( token::Type::SELECT, token::Class::KEYWORD, "SELECT" ),
                                token::Token ( token::Type::FROM, token::Class::KEYWORD, "FROM" ),
                                token::Token ( token::Type::WHERE, token::Class::KEYWORD, "WHERE" ),
                                token::Token ( token::Type::AND, token::Class::LOGICAL_OPERATOR, "AND" ),
                                token::Token ( token::Type::OR, token::Class::LOGICAL_OPERATOR, "OR" ),
                                token::Token ( token::Type::BOOLEAN, token::Class::KEYWORD, "TRUE" ),
                                token::Token ( token::Type::BOOLEAN, token::Class::KEYWORD, "FALSE" ),
                                token::Token ( token::Type::COMMA, token::Class::SPECIAL, "," ),
                                token::Token ( token::Type::MULTIPLICATION, token::Class::ARITHMETIC_OPERATOR, "*" ),
                                token::Token ( token::Type::PLUS, token::Class::ARITHMETIC_OPERATOR, "+" ),
                                token::Token ( token::Type::MINUS, token::Class::ARITHMETIC_OPERATOR, "-" ),
                                token::Token ( token::Type::DIVIDE, token::Class::ARITHMETIC_OPERATOR, "/" ),
                                token::Token ( token::Type::EQUAL, token::Class::RELATIONAL_OPERATOR, "=" ),
                                token::Token ( token::Type::NOT_EQUAL, token::Class::RELATIONAL_OPERATOR, "!=" ),
                                token::Token ( token::Type::LEFT_PARENTHESIS, token::Class::SPECIAL, "(" ),
                                token::Token ( token::Type::RIGHT_PARENTHESIS, token::Class::SPECIAL, ")" ),
                                token::Token ( token::Type::AS, token::Class::KEYWORD, "AS" ),
                                token::Token ( token::Type::UPPER, token::Class::FUNCTION, "UPPER" ),
                                token::Token ( token::Type::LOWER, token::Class::FUNCTION, "LOWER" ),
                                token::Token ( token::Type::IDENTIFIER, token::Class::ENTRY, "NAME" ),
                                token::Token ( token::Type::IDENTIFIER, token::Class::ENTRY, "SIZE" ),
                                token::Token ( token::Type::IDENTIFIER, token::Class::ENTRY, "RELATIVE_PATH" ),
                                token::Token ( token::Type::IDENTIFIER, token::Class::ENTRY, "CREATION" ),
                                token::Token ( token::Type::NOT, token::Class::LOGICAL_OPERATOR, "NOT" ),
                                token::Token ( token::Type::GREATER_THAN, token::Class::RELATIONAL_OPERATOR, ">" ),
                                token::Token ( token::Type::GREATER_THAN_OR_EQUAL, token::Class::RELATIONAL_OPERATOR, ">=" ),
                                token::Token ( token::Type::LESS_THAN, token::Class::RELATIONAL_OPERATOR, "<" ),
                                token::Token ( token::Type::LESS_THAN_OR_EQUAL, token::Class::RELATIONAL_OPERATOR, "<=" ),
                                token::Token ( token::Type::LIKE, token::Class::RELATIONAL_OPERATOR, "LIKE" ) };

token::Token build_token ( const string &value );

Lexer::Lexer ( const string input ) : input ( normalize ( input ) )
{
    this->current_index = 0;
}

bool Lexer::has_next () const
{
    return this->current_index < input.size ();
}

token::Token Lexer::eat ()
{
    if ( !this->has_next () )
        {
            return token::Token ( token::Type::INVALID, token::Class::SPECIAL, "END" );
        }

    string     value                = "";
    const auto start                = this->current_index;
    bool       inside_double_quotes = false;
    while ( this->has_next () )
        {
            const auto c = this->input[ this->current_index ];
            this->current_index++;
            if ( c == '\"' )
                {
                    inside_double_quotes = !inside_double_quotes;
                }
            else if ( c == ' ' && !inside_double_quotes )
                {
                    value = this->input.substr ( start, this->current_index - start - 1 );
                    break;
                }
        }

    this->previous_indexes.push ( start );
    return build_token ( value );
}

token::Token Lexer::next ()
{
    return this->eat ();
}

token::Token Lexer::peek ()
{
    const auto previous_index = this->current_index;
    const auto token          = this->eat ();
    this->current_index       = previous_index;
    this->previous_indexes.pop ();
    return token;
}

token::Token Lexer::previous ()
{
    if ( this->previous_indexes.empty () )
        {
            return token::Token ( token::Type::INVALID, token::Class::SPECIAL, "Invalid Token" );
        }
    this->current_index = this->previous_indexes.top ();
    this->previous_indexes.pop ();
    return this->eat ();
}

token::Token build_token ( const string &value )
{
    {
        string uppercase_value = value;
        transform ( uppercase_value.begin (), uppercase_value.end (), uppercase_value.begin (), ::toupper );
        for ( auto &token : TOKENS )
            {
                if ( token.get_value () == uppercase_value )
                    {
                        return token;
                    }
            }
    }
    if ( regex_search ( value, number ) )
        {
            return token::Token ( token::Type::NUMBER, token::Class::ENTRY, value );
        }
    if ( value[ 0 ] == '"' && value[ value.size () - 1 ] == '"' )
        {
            // TODO: HANDLE INVALID INPUT
            auto temp = value.substr ( 1, value.size () - 2 );
            return token::Token ( token::Type::STRING, token::Class::ENTRY, temp );
        }
    throw "Invalid Token";
}

void Lexer::puke ()
{
    uint16_t previous = 0;
    if ( this->previous_indexes.size () != 0 )
        {
            previous = this->previous_indexes.top ();
            this->previous_indexes.pop ();
        }
    this->current_index = previous;
}