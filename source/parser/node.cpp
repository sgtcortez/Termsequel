#include "termsequel/parser/node.hpp"
#include "termsequel/evaluator/evaluator.hpp"
#include "termsequel/lexer/token.hpp"
#include <memory>

using namespace termsequel;
using namespace parser;

using namespace std;

bool Node::operator== ( const Node &other ) const
{
    const auto &left_type  = typeid ( *this );
    const auto &right_type = typeid ( other );
    return left_type == right_type && this->serialize () == other.serialize ();
}

TableNode::TableNode ( const std::string value ) : value ( value ), type ( Type::UNKNOWN )
{
}

NodeType TableNode::get_type () const
{
    return NodeType::TABLE;
}

string TableNode::serialize () const
{
    return this->value;
}

NodeType SelectStatementNode::get_type () const
{
    return NodeType::SELECT_STATEMENT;
}

string SelectStatementNode::serialize () const
{
    string output = "SELECT \n";
    for ( const auto &column : this->columns )
        {
            output += "\t" + column->serialize () + "\n";
        }
    output += "FROM \n\t" + this->table->serialize () + "\n";
    if ( this->filter == nullptr )
        {
            return output;
        }
    output += "WHERE\n\t" + this->filter->serialize ();
    return output;
}

ColumnNode::ColumnNode ( ValueNode *value, const string name ) : value ( value ), name ( name )
{
}

NodeType ColumnNode::get_type () const
{
    return NodeType::COLUMN;
}

string ColumnNode::serialize () const
{
    return this->value->serialize () + " AS " + this->name;
}

std::string ColumnNode::evaluate () const
{
    return this->value->evaluate ();
}

LeafNode::LeafNode ( const NodeType type ) : type ( type )
{
}

LeafNode::LeafNode ( const string value, const NodeType type ) : value ( value ), type ( type )
{
}

NodeType LeafNode::get_type () const
{
    return this->type;
}

string LeafNode::serialize () const
{
    return this->value;
}

std::string LeafNode::evaluate () const
{
    return this->value;
}

PlaceholderNode::PlaceholderNode ( const string identifier_name ) : identifier_name ( identifier_name )
{
    value = nullptr;
}

NodeType PlaceholderNode::get_type () const
{
    if ( this->value == nullptr )
        {
            return NodeType::IDENTIFIER;
        }
    return this->value->get_type ();
}

string PlaceholderNode::serialize () const
{
    if ( this->value == nullptr || this->value->value.empty () )
        {
            return this->identifier_name;
        }
    return this->value->serialize ();
}

std::string PlaceholderNode::evaluate () const
{
    return this->value->evaluate ();
}

FunctionNode::FunctionNode ( const token::Token function, ValueNode *value ) : function ( function ), value ( value )
{
}

NodeType FunctionNode::get_type () const
{
    return NodeType::FUNCTION;
}

string FunctionNode::serialize () const
{
    return this->function.get_value () + "(" + this->value->serialize () + ")";
}

string FunctionNode::evaluate () const
{
    return evaluator::evaluate ( this->value->evaluate (), this->function );
}

UnaryExpressionNode::UnaryExpressionNode ( ValueNode *value, const token::Token operation ) : value ( value ), operation ( operation )
{
}

NodeType UnaryExpressionNode::get_type () const
{
    return NodeType::UNARY_EXPRESSION;
}

string UnaryExpressionNode::serialize () const
{
    return "(" + this->operation.get_value () + " " + this->value->serialize () + ")";
}

string UnaryExpressionNode::evaluate () const
{
    return evaluator::evaluate ( this->value->evaluate (), this->operation );
}

BinaryExpressionNode::BinaryExpressionNode ( ValueNode *left, token::Token operation, ValueNode *right )
    : left ( unique_ptr<ValueNode> ( left ) ), operation ( operation ), right ( unique_ptr<ValueNode> ( right ) )
{
}

NodeType BinaryExpressionNode::get_type () const
{
    return NodeType::BINARY_EXPRESSION;
}

string BinaryExpressionNode::serialize () const
{
    const auto left_serialize  = this->left->serialize ();
    const auto right_serialize = this->right->serialize ();
    return "(" + left_serialize + " " + this->operation.get_value () + " " + right_serialize + ")";
}

std::string BinaryExpressionNode::evaluate () const
{
    const auto left_value  = this->left->evaluate ();
    const auto right_value = this->right->evaluate ();
    return evaluator::evaluate ( left_value, this->operation, right_value );
}

ValueNode *parser::node_factory ( const token::Token token )
{
    switch ( token.get_type () )
        {
        case token::Type::NUMBER:
            return new LeafNode ( token.get_value (), NodeType::NUMBER );
        case token::Type::BOOLEAN:
            return new LeafNode ( token.get_value (), NodeType::BOOLEAN );
        case token::Type::STRING:
            return new LeafNode ( token.get_value (), NodeType::STRING );
        case token::Type::IDENTIFIER:
            return new PlaceholderNode ( token.get_value () );
        default:
            throw std::runtime_error ( "Unknown token type" );
        }
}
