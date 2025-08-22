#include "termsequel/evaluator/evaluator.hpp"

#include <algorithm>
#include <cctype>
#include <string>

using namespace std;

int64_t arithmetic_operations ( int64_t left, const termsequel::token::Token &operation, int64_t right );
bool    logical_operations ( bool left, const termsequel::token::Token &operation, bool right );
bool    relational_operations ( string left, const termsequel::token::Token &operation, string right );

string termsequel::evaluator::evaluate ( const string &left, const token::Token &operation, const string &right )
{
    switch ( operation.get_class () )
        {
        case token::Class::ARITHMETIC_OPERATOR:
            return to_string ( arithmetic_operations ( stoi ( left ), operation, stoi ( right ) ) );
        case token::Class::LOGICAL_OPERATOR:
            return to_string ( logical_operations ( left == "1", operation, right == "1" ) );
        case token::Class::RELATIONAL_OPERATOR:
            return to_string ( relational_operations ( left, operation, right ) );
        default:
            throw "Invalid operation";
        }
}

std::string termsequel::evaluator::evaluate ( const std::string &value, const token::Token &operation )
{
    switch ( operation.get_class () )
        {
        case token::Class::ARITHMETIC_OPERATOR:
            return "-" + value;
        case token::Class::FUNCTION:
            {
                if ( operation.get_type () == token::Type::UPPER )
                    {
                        string result = value;
                        // result.reserve(value.size());
                        //  This just converts the value to uppercase
                        transform ( value.begin (), value.end (), result.begin (), ::toupper );
                        return result;
                    }
                else if ( operation.get_type () == token::Type::LOWER )
                    {
                        string result = value;
                        // result.reserve(value.size());
                        //  This just converts the value to uppercase
                        transform ( value.begin (), value.end (), result.begin (), ::tolower );
                        return result;
                    }
                throw "Invalid operation";
            }
        case token::Class::LOGICAL_OPERATOR:
            {
                if ( operation.get_type () == token::Type::NOT )
                    {
                        return value == "1" ? "0" : "1";
                    }
            }
            throw "Invalid operation";
        default:
            throw "Invalid operation";
        }
}

int64_t arithmetic_operations ( int64_t left, const termsequel::token::Token &operation, int64_t right )
{
    switch ( operation.get_type () )
        {
        case termsequel::token::Type::PLUS:
            return left + right;
        case termsequel::token::Type::MINUS:
            return left - right;
        case termsequel::token::Type::MULTIPLICATION:
            return left * right;
        case termsequel::token::Type::DIVIDE:
            return left / right;
        default:
            throw "Invalid Arithmetic operation";
        }
}

bool logical_operations ( bool left, const termsequel::token::Token &operation, bool right )
{
    switch ( operation.get_type () )
        {
        case termsequel::token::Type::AND:
            return left && right;
        case termsequel::token::Type::OR:
            return left || right;
        default:
            throw "Invalid Logical operation";
        }
};

bool relational_operations ( string left, const termsequel::token::Token &operation, string right )
{
    switch ( operation.get_type () )
        {
        case termsequel::token::Type::EQUAL:
            return left == right;
        case termsequel::token::Type::NOT_EQUAL:
            return left != right;
        case termsequel::token::Type::GREATER_THAN:
            return stoi ( left ) > stoi ( right );
        case termsequel::token::Type::GREATER_THAN_OR_EQUAL:
            return stoi ( left ) >= stoi ( right );
        case termsequel::token::Type::LESS_THAN:
            return stoi ( left ) < stoi ( right );
        case termsequel::token::Type::LESS_THAN_OR_EQUAL:
            return stoi ( left ) <= stoi ( right );
        case termsequel::token::Type::LIKE:
            return left.find ( right ) != string::npos;
        default:
            throw "error";
        }
}
