#include "termsequel/lexer/token.hpp"

using namespace termsequel;
using namespace token;

using namespace std;

Token::Token ( const Type type, const Class clazz, const string value ) : type ( type ), clazz ( clazz ), value ( value )
{
}

bool Token::operator== ( const Token &other ) const
{
    return this->type == other.type && this->value == other.value;
}

Type Token::get_type () const
{
    return this->type;
}

Class Token::get_class () const
{
    return this->clazz;
}

const string Token::get_value () const
{
    return this->value;
}