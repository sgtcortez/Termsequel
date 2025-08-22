#include "termsequel/analyzer/analyzer.hpp"
#include "termsequel/definitions.hpp"
#include "termsequel/log.hpp"
#include "termsequel/parser/node.hpp"

#include <cstdint>
#include <memory>
#include <string>

#ifdef __linux__
#include <sys/stat.h>
#endif

using namespace termsequel;
using namespace analyzer;

using namespace std;

void infer_data_types_on_identifier_nodes ( parser::Node *node );
void infer_source ( parser::TableNode *table );

void Analyzer::analyze ( parser::StatementNode *abstract_syntax_tree ) const
{
    if ( !( abstract_syntax_tree->get_type () == parser::NodeType::SELECT_STATEMENT ) )
        {
            LOG_DEBUG ( "For now, only SELECT statement is supported!" );
            throw "Error";
        }

    auto select = static_cast<parser::SelectStatementNode *> ( abstract_syntax_tree );
    infer_source ( select->table.get () );
    for ( auto &column : select->columns )
        {
            infer_data_types_on_identifier_nodes ( column.get () );
            // analyze_expression(*column);
        }
    if ( select->filter == nullptr )
        {
            return;
        }
    infer_data_types_on_identifier_nodes ( select->filter.get () );
    // analyze_expression(*(select->filter));
}

void infer_data_types_on_identifier_nodes ( parser::Node *node )
{
    if ( node->get_type () == parser::NodeType::IDENTIFIER )
        {
            auto        idnode = static_cast<parser::PlaceholderNode *> ( node );
            const auto &column = definitions::Column::get ( idnode->identifier_name );
            if ( column == definitions::Column::INVALID )
                {
                    LOG_DEBUG ( "Column not found!" );
                    throw "Error";
                }
            auto token_type = column.type == definitions::ColumnDataType::NUMERIC ? parser::NodeType::NUMBER : parser::NodeType::STRING;
            idnode->value.release ();
            idnode->value = std::make_unique<parser::LeafNode> ( token_type );
            return;
        };
    if ( node->get_type () == parser::NodeType::COLUMN )
        {
            auto idnode = static_cast<parser::ColumnNode *> ( node );
            infer_data_types_on_identifier_nodes ( idnode->value.get () );
            return;
        }

    if ( node->get_type () == parser::NodeType::FUNCTION )
        {
            auto idnode = static_cast<parser::FunctionNode *> ( node );
            infer_data_types_on_identifier_nodes ( idnode->value.get () );
            return;
        }

    if ( node->get_type () == parser::NodeType::BINARY_EXPRESSION )
        {
            auto binary = static_cast<parser::BinaryExpressionNode *> ( node );
            infer_data_types_on_identifier_nodes ( binary->left.get () );
            infer_data_types_on_identifier_nodes ( binary->right.get () );
            return;
        }

    if ( node->get_type () == parser::NodeType::UNARY_EXPRESSION )
        {
            auto unary = dynamic_cast<parser::UnaryExpressionNode *> ( node );
            infer_data_types_on_identifier_nodes ( unary->value.get () );
            return;
        }
}

void infer_source ( parser::TableNode *table )
{
#ifdef __linux__

    struct stat buffer;
    int32_t     result = stat ( table->value.c_str (), &buffer );
    if ( result < 0 )
        {
            LOG_DEBUG ( "Could not find the file/directory located at: " << table->value );
            throw "Error";
        }

    switch ( buffer.st_mode & S_IFMT )
        {
        case S_IFDIR:
            table->type = parser::TableNode::Type::DIRECTORY;
            break;
        case S_IFREG:
            table->type = parser::TableNode::Type::REGULAR_FILE;
            break;
        default:
            throw "Unsupported File type!";
        }

#endif
}
