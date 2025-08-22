#ifndef __TERMSEQUEL_NODE__
#define __TERMSEQUEL_NODE__

#include "termsequel/lexer/token.hpp"
#include <memory>
#include <string>
#include <vector>

namespace termsequel {
namespace parser {
enum class NodeType : std::uint8_t
{
    UNKNOWN           = 0,
    FUNCTION          = 1,
    NUMBER            = 2,
    STRING            = 3,
    IDENTIFIER        = 4,
    BINARY_EXPRESSION = 5,
    UNARY_EXPRESSION  = 6,
    DATE              = 7,
    COLUMN            = 8,
    TABLE             = 9,
    SELECT_STATEMENT  = 10,
    BOOLEAN           = 12,
};

struct Node
{
    virtual ~Node ()                       = default;
    virtual NodeType    get_type () const  = 0;
    virtual std::string serialize () const = 0;
    virtual bool        operator== ( const Node &other ) const;
};

struct ValueNode : public Node
{
    ~ValueNode () override                = default;
    virtual std::string evaluate () const = 0;
};

struct TableNode final : public Node
{
    const std::string value;
    enum class Type : std::uint8_t
    {
        UNKNOWN      = 0,
        DIRECTORY    = 1,
        REGULAR_FILE = 2,
    } type;
    TableNode ( const std::string value );
    NodeType    get_type () const override;
    std::string serialize () const override;
};

struct ColumnNode : public ValueNode
{
    const std::unique_ptr<ValueNode> value;
    const std::string                name;
    ColumnNode ( ValueNode *value, std::string name );
    NodeType    get_type () const override;
    std::string serialize () const override;
    std::string evaluate () const override;
};

struct StatementNode : public Node
{
    std::unique_ptr<TableNode> table;
    virtual ~StatementNode () = default;
};

struct SelectStatementNode : public StatementNode
{
    std::vector<std::unique_ptr<ColumnNode>> columns;
    std::unique_ptr<ValueNode>               filter;

    NodeType    get_type () const override;
    std::string serialize () const override;
};

struct LeafNode : ValueNode
{
    std::string    value;
    const NodeType type;
    LeafNode ( const std::string value, const NodeType type );
    LeafNode ( const NodeType type );
    NodeType    get_type () const override;
    std::string serialize () const override;
    ~LeafNode () = default;

    std::string evaluate () const override;
};

struct PlaceholderNode : ValueNode
{
    const std::string         identifier_name;
    std::unique_ptr<LeafNode> value;
    PlaceholderNode ( const std::string identifier_name );
    NodeType    get_type () const override;
    std::string serialize () const override;
    std::string evaluate () const override;
};

struct FunctionNode : ValueNode
{
    const token::Token         function;
    std::unique_ptr<ValueNode> value;
    FunctionNode ( const token::Token function, ValueNode *value );
    NodeType    get_type () const override;
    std::string serialize () const override;
    std::string evaluate () const override;
};

struct UnaryExpressionNode : public ValueNode
{
    const std::unique_ptr<ValueNode> value;
    const token::Token               operation;
    UnaryExpressionNode ( ValueNode *value, const token::Token operation );
    NodeType    get_type () const override;
    std::string serialize () const override;
    std::string evaluate () const override;
};

struct BinaryExpressionNode : public ValueNode
{
    const std::unique_ptr<ValueNode> left;
    const token::Token               operation;
    const std::unique_ptr<ValueNode> right;
    BinaryExpressionNode ( ValueNode *left, token::Token operation, ValueNode *right );
    NodeType    get_type () const override;
    std::string serialize () const override;
    std::string evaluate () const override;
};

ValueNode *node_factory ( const token::Token token );
}; // namespace parser
}; // namespace termsequel

#endif
