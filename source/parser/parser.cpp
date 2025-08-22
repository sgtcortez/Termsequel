#include "termsequel/parser/parser.hpp"
#include "termsequel/lexer/lexer.hpp"
#include "termsequel/lexer/token.hpp"
#include "termsequel/log.hpp"
#include "termsequel/parser/node.hpp"

#include <cstddef>
#include <iterator>
#include <memory>
#include <sstream>
#include <stack>
#include <unordered_map>

using namespace termsequel;
using namespace parser;

using namespace std;

enum class AssociationType : uint8_t
{
    LEFT  = 1,
    RIGHT = 2
};

struct Precedence
{
    const int8_t          value;
    const AssociationType association;

    Precedence ( int8_t value, AssociationType association ) : value ( value ), association ( association ){};
    bool operator> ( const Precedence &other ) const
    {
        return value > other.value;
    }

    bool operator>= ( const Precedence &other ) const
    {
        return operator> ( other ) || ( this->value == other.value && other.association == AssociationType::LEFT );
    }
};

unordered_map<token::Type, Precedence> OPERATOR_PRECEDENCE = {

    { token::Type::EQUAL, { 0, AssociationType::LEFT } },          { token::Type::OR, { -1, AssociationType::LEFT } },
    { token::Type::AND, { -1, AssociationType::LEFT } },           { token::Type::NOT_EQUAL, { 0, AssociationType::LEFT } },
    { token::Type::GREATER_THAN, { 0, AssociationType::LEFT } },   { token::Type::GREATER_THAN_OR_EQUAL, { 0, AssociationType::LEFT } },
    { token::Type::LESS_THAN, { 0, AssociationType::LEFT } },      { token::Type::LESS_THAN_OR_EQUAL, { 0, AssociationType::LEFT } },
    { token::Type::PLUS, { 1, AssociationType::LEFT } },           { token::Type::MINUS, { 1, AssociationType::LEFT } },
    { token::Type::MULTIPLICATION, { 2, AssociationType::LEFT } }, { token::Type::DIVIDE, { 2, AssociationType::LEFT } },
    { token::Type::UNARY_MINUS, { 10, AssociationType::RIGHT } },  { token::Type::UPPER, { 10, AssociationType::RIGHT } },
    { token::Type::LIKE, { 10, AssociationType::RIGHT } },         { token::Type::NOT, { 10, AssociationType::RIGHT } } };

unique_ptr<ColumnNode> parse_column ( lexer::Lexer &lexer, uint8_t column_index );
ValueNode             *parse_condition ( lexer::Lexer &lexer );
void                   handle_operator_precedence ( const token::Token &token, stack<ValueNode *> &operands, stack<token::Token> &operators );
void                   handle_end_stream ( stack<ValueNode *> &operands, stack<token::Token> &operators );
void                   handle_operation ( const token::Token &operation, stack<ValueNode *> &operands );

unique_ptr<StatementNode> Parser::parse ( lexer::Lexer &lexer ) const
{
    if ( lexer.next ().get_type () != token::Type::SELECT )
        {
            LOG_ERROR ( "Only SELECT statement is supported by now!" );
            throw "Only SELECT statement is supported by now!";
        }
    SelectStatementNode *select       = new SelectStatementNode;
    uint8_t              column_index = 1;

    while ( true )
        {
            const auto token = lexer.next ();
            if ( token.get_type () == token::Type::FROM )
                {
                    break;
                }
            if ( token.get_type () == token::Type::COMMA )
                {
                    continue;
                }
            // Puke the consumed token
            lexer.puke ();
            select->columns.push_back ( parse_column ( lexer, column_index++ ) );
        }

    const auto from_path_token = lexer.next ();
    if ( from_path_token.get_type () != token::Type::STRING )
        {
            LOG_ERROR ( "Expectd a STRING to get the from \"location\"!" );
            throw "INvalid!";
        }
    select->table = unique_ptr<TableNode> ( new TableNode ( from_path_token.get_value () ) );

    if ( lexer.has_next () )
        {
            if ( lexer.next ().get_type () != token::Type::WHERE )
                {
                    LOG_ERROR ( "Expected the where keyword!" );
                    throw "Expected \"WHERE\"";
                }
            select->filter = unique_ptr<ValueNode> ( parse_condition ( lexer ) );
        }
    return unique_ptr<SelectStatementNode> ( select );
}

unique_ptr<ColumnNode> parse_column ( lexer::Lexer &lexer, uint8_t column_index )
{
    stack<ValueNode *>  operands;
    stack<token::Token> operators;

    set<token::Type> accepted = { token::Type::UPPER, token::Type::LOWER, token::Type::MINUS, token::Type::NUMBER, token::Type::STRING, token::Type::IDENTIFIER };

    bool running = true;

    string name = "COLUMN_" + to_string ( column_index );

    while ( running )
        {
            const auto previous_token = lexer.previous ();
            const auto token          = lexer.next ();
            if ( accepted.find ( token.get_type () ) == accepted.end () )
                {
                    LOG_ERROR ( "Unexpeted token found. Query is not well formed!" );
                    throw "Query is not wellformed!";
                }

            switch ( token.get_type () )
                {
                case token::Type::UPPER:
                case token::Type::LOWER:
                    {
                        // The function upper syntax its:
                        // UPPER(STRING|PLACEHOLDER)
                        operators.push ( token );
                        accepted = {
                            token::Type::LEFT_PARENTHESIS,
                            token::Type::STRING,
                        };
                        break;
                    }

                case token::Type::LEFT_PARENTHESIS:
                    {
                        operators.push ( token );
                        if ( previous_token.get_class () == token::Class::FUNCTION )
                            {
                                /*
                                 * In this case, the brace its used
                                 * to identify the function
                                 * arguments
                                 */
                                accepted = {
                                    token::Type::IDENTIFIER,
                                    token::Type::NUMBER,
                                    token::Type::UPPER,
                                    token::Type::LOWER,
                                };
                            }
                        break;
                    }
                case token::Type::RIGHT_PARENTHESIS:
                    {
                        handle_operator_precedence ( token, operands, operators );
                        break;
                    }
                case token::Type::MINUS:
                    {
                        if ( previous_token.get_class () == token::Class::KEYWORD || previous_token.get_class () == token::Class::LOGICAL_OPERATOR ||
                             previous_token.get_class () == token::Class::ARITHMETIC_OPERATOR || previous_token.get_class () == token::Class::RELATIONAL_OPERATOR ||
                             previous_token.get_type () == token::Type::LEFT_PARENTHESIS )
                            {
                                // minus unary operator
                                operators.emplace ( token::Type::UNARY_MINUS, token::Class::ARITHMETIC_OPERATOR, "-" );

                                accepted = {
                                    token::Type::IDENTIFIER,
                                    token::Type::NUMBER,
                                    token::Type::LEFT_PARENTHESIS,
                                };
                                break;
                            }
                        [[fallthrough]];
                    }
                case token::Type::PLUS:
                case token::Type::DIVIDE:
                case token::Type::MULTIPLICATION:
                    {
                        handle_operator_precedence ( token, operands, operators );
                        accepted = {
                            token::Type::IDENTIFIER, token::Type::NUMBER, token::Type::LEFT_PARENTHESIS, token::Type::RIGHT_PARENTHESIS, token::Type::MINUS,
                        };
                        break;
                    }
                case token::Type::IDENTIFIER:
                case token::Type::STRING:
                case token::Type::NUMBER:
                    {
                        if ( token.get_type () == token::Type::STRING && previous_token.get_type () == token::Type::AS )
                            {
                                /*
                                 * The ALIAS its a special keyword,
                                 */
                                accepted = {
                                    token::Type::COMMA,
                                    token::Type::FROM,
                                };
                                name = token.get_value ();
                                break;
                            }

                        accepted = { token::Type::PLUS,  token::Type::MINUS, token::Type::MULTIPLICATION,   token::Type::DIVIDE,
                                     token::Type::COMMA, token::Type::FROM,  token::Type::LEFT_PARENTHESIS, token::Type::RIGHT_PARENTHESIS,
                                     token::Type::AS };
                        operands.push ( node_factory ( token ) );
                        break;
                    }
                case token::Type::COMMA:
                case token::Type::FROM:
                    {
                        running = false;
                        break;
                    }
                case token::Type::AS:
                    {
                        accepted = { token::Type::STRING };
                        break;
                    }
                default:
                    LOG_ERROR ( "Unexpected token" );
                    throw "Unexpected token";
                }
        }

    // we need to puke the last consumed token("," OR "FROM")
    // so, we can easily iterate
    lexer.puke ();

    handle_end_stream ( operands, operators );
    const auto top = operands.top ();
    operands.pop ();

    if ( !operands.empty () || !operators.empty () )
        {
            LOG_ERROR ( "Invalid query!" );
            throw "Error";
        }
    return make_unique<ColumnNode> ( top, name );
}

ValueNode *parse_condition ( lexer::Lexer &lexer )
{

    stack<ValueNode *>  operands;
    stack<token::Token> operators;

    set<token::Type> accepted = {
        token::Type::LEFT_PARENTHESIS, token::Type::UPPER, token::Type::LOWER, token::Type::MINUS, token::Type::NUMBER, token::Type::STRING, token::Type::IDENTIFIER, token::Type::NOT,
    };

    while ( lexer.has_next () )
        {
            const auto previous_token = lexer.previous ();
            const auto token          = lexer.next ();

            if ( accepted.find ( token.get_type () ) == accepted.end () )
                {
                    LOG_ERROR ( "Unexpeted token found. Query is not well formed! Expected tokens were: " );
                    throw "Query is not wellformed!";
                }

            switch ( token.get_type () )
                {
                case token::Type::NOT:
                    {
                        accepted = {
                            token::Type::EQUAL, token::Type::NOT_EQUAL, token::Type::GREATER_THAN, token::Type::LESS_THAN, token::Type::GREATER_THAN_OR_EQUAL, token::Type::LESS_THAN_OR_EQUAL,
                            token::Type::UPPER, token::Type::LOWER,     token::Type::LIKE,
                        };
                        operators.push ( token );
                        break;
                    }
                case token::Type::LIKE:
                    {
                        accepted = {
                            token::Type::STRING,
                            token::Type::UPPER,
                            token::Type::LOWER,
                        };
                        operators.push ( token );
                        break;
                    }
                case token::Type::UPPER:
                case token::Type::LOWER:
                    {
                        // The function upper syntax its:
                        // UPPER(STRING|PLACEHOLDER)
                        operators.push ( token );
                        accepted = {
                            token::Type::LEFT_PARENTHESIS,
                            token::Type::STRING,
                        };
                        break;
                    }

                case token::Type::LEFT_PARENTHESIS:
                    {
                        operators.push ( token );
                        if ( previous_token.get_class () == token::Class::FUNCTION )
                            {
                                /*
                                 * In this case, the brace its used
                                 * to identify the function
                                 * arguments
                                 */
                                accepted = { token::Type::IDENTIFIER, token::Type::NUMBER, token::Type::UPPER, token::Type::LOWER, token::Type::STRING };
                            }
                        break;
                    }
                case token::Type::RIGHT_PARENTHESIS:
                    {
                        handle_operator_precedence ( token, operands, operators );
                        break;
                    }
                case token::Type::MINUS:
                    {
                        if ( previous_token.get_class () == token::Class::KEYWORD || previous_token.get_class () == token::Class::LOGICAL_OPERATOR ||
                             previous_token.get_class () == token::Class::ARITHMETIC_OPERATOR || previous_token.get_class () == token::Class::RELATIONAL_OPERATOR ||
                             previous_token.get_type () == token::Type::LEFT_PARENTHESIS )
                            {
                                // minus unary operator
                                operators.emplace ( token::Type::UNARY_MINUS, token::Class::ARITHMETIC_OPERATOR, "-" );

                                accepted = {
                                    token::Type::IDENTIFIER,
                                    token::Type::NUMBER,
                                    token::Type::LEFT_PARENTHESIS,
                                };
                                break;
                            }
                        [[fallthrough]];
                    }

                case token::Type::PLUS:
                case token::Type::DIVIDE:
                case token::Type::MULTIPLICATION:
                case token::Type::EQUAL:
                case token::Type::NOT_EQUAL:
                case token::Type::GREATER_THAN:
                case token::Type::GREATER_THAN_OR_EQUAL:
                case token::Type::LESS_THAN:
                case token::Type::LESS_THAN_OR_EQUAL:
                case token::Type::AND:
                case token::Type::OR:
                    {
                        handle_operator_precedence ( token, operands, operators );
                        accepted = {
                            token::Type::IDENTIFIER, token::Type::NUMBER, token::Type::LEFT_PARENTHESIS, token::Type::RIGHT_PARENTHESIS, token::Type::MINUS, token::Type::STRING,
                        };
                        if ( token.get_type () == token::Type::OR || token.get_type () == token::Type::AND )
                            {
                                accepted.insert ( token::Type::UPPER );
                                accepted.insert ( token::Type::LOWER );
                            }
                        break;
                    }
                case token::Type::IDENTIFIER:
                case token::Type::STRING:
                case token::Type::NUMBER:
                    {
                        accepted = {
                            token::Type::PLUS,
                            token::Type::MINUS,
                            token::Type::MULTIPLICATION,
                            token::Type::DIVIDE,
                            token::Type::EQUAL,
                            token::Type::NOT_EQUAL,
                            token::Type::GREATER_THAN,
                            token::Type::GREATER_THAN_OR_EQUAL,
                            token::Type::LESS_THAN,
                            token::Type::LESS_THAN_OR_EQUAL,
                            token::Type::LEFT_PARENTHESIS,
                            token::Type::RIGHT_PARENTHESIS,
                            token::Type::OR,
                            token::Type::AND,
                        };

                        if ( token.get_type () == token::Type::IDENTIFIER )
                            {
                                accepted.insert ( token::Type::LIKE );
                                accepted.insert ( token::Type::NOT );
                            }

                        operands.push ( node_factory ( token ) );
                        break;
                    }
                default:
                    throw "Unexpected token";
                }
        }

    handle_end_stream ( operands, operators );
    const auto top = operands.top ();
    operands.pop ();
    if ( !operands.empty () || !operators.empty () )
        {
            LOG_ERROR ( "Parse Condition is not well formed!" );
            throw "Error";
        }
    return top;
}

void handle_operator_precedence ( const token::Token &token, stack<ValueNode *> &operands, stack<token::Token> &operators )
{
    // https://en.wikipedia.org/wiki/Shunting_yard_algorithm
    if ( operators.empty () )
        {
            operators.push ( token );
            return;
        }
    const bool is_noop = token.get_type () == token::Type::RIGHT_PARENTHESIS;

    while ( true )
        {
            if ( operators.empty () )
                {
                    break;
                }
            const auto top_operator = operators.top ();

            if ( top_operator.get_type () == token::Type::LEFT_PARENTHESIS )
                {
                    break;
                }
            if ( is_noop || OPERATOR_PRECEDENCE.at ( top_operator.get_type () ) >= OPERATOR_PRECEDENCE.at ( token.get_type () ) )
                {
                    operators.pop ();
                    handle_operation ( top_operator, operands );
                    continue;
                }

            // new operator has a bigger precedence than the current one
            break;
        }

    if ( !is_noop )
        {
            operators.push ( token );
        }
    else
        {
            // discards the left brace operator
            operators.pop ();

            if ( !operators.empty () && operators.top ().get_class () == token::Class::FUNCTION )
                {
                    const auto top_operator = operators.top ();
                    operators.pop ();
                    handle_operation ( top_operator, operands );
                }
        }
}

void handle_end_stream ( stack<ValueNode *> &operands, stack<token::Token> &operators )
{
    while ( !operators.empty () )
        {
            const auto op = operators.top ();
            operators.pop ();
            handle_operation ( op, operands );
        }
}

void handle_operation ( const token::Token &operation, stack<ValueNode *> &operands )
{
    auto first_operand = operands.top ();
    operands.pop ();
    ValueNode *value = nullptr;

    if ( operation.get_class () == token::Class::ARITHMETIC_OPERATOR || operation.get_class () == token::Class::LOGICAL_OPERATOR || operation.get_class () == token::Class::RELATIONAL_OPERATOR )
        {
            if ( operation.get_type () == token::Type::UNARY_MINUS || operation.get_type () == token::Type::NOT )
                {
                    // handles unary operations
                    value = new UnaryExpressionNode ( first_operand, operation );
                }
            else
                {
                    // handles binary operation
                    const auto left = operands.top ();
                    operands.pop ();
                    value = new BinaryExpressionNode ( left, operation, first_operand );
                }
        }
    else if ( operation.get_class () == token::Class::FUNCTION )
        {
            // by now, we support only single argument functions
            value = new FunctionNode ( operation, first_operand );
        }
    operands.push ( value );
}