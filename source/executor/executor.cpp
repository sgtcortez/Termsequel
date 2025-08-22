#include "termsequel/executor/executor.hpp"

#include "termsequel/analyzer/analyzer.hpp"
#include "termsequel/lexer/lexer.hpp"
#include "termsequel/log.hpp"
#include "termsequel/parser/node.hpp"
#include "termsequel/parser/parser.hpp"
#include "termsequel/system/system.hpp"
#include "termsequel/view/view.hpp"

#include <thread>
#include <unordered_map>

using namespace termsequel;
using namespace executor;

using namespace std;

unordered_map<string, vector<parser::PlaceholderNode *>> get_placeholders ( parser::Node *root );
void merge_dictionaries ( unordered_map<string, vector<parser::PlaceholderNode *>> &left, unordered_map<string, vector<parser::PlaceholderNode *>> right );
void append_data_to_placeholders ( const system::FileInformation &file, unordered_map<string, vector<parser::PlaceholderNode *>> &nodes );
bool filter ( parser::Node *condition );

vector<view::ColumnView> map_view ();
void                     set_results ();

view::View                                              *view_v;
parser::SelectStatementNode                             *select_t;
unordered_map<string, vector<parser::PlaceholderNode *>> placehoders_nodes;

void file_callback ( system::FileInformation file )
{
    append_data_to_placeholders ( file, placehoders_nodes );
    if ( !filter ( select_t->filter.get () ) )
        {
            // LOG_DEBUG("File: " << file.name << " was removed due to
            // conditions that were not met!");
            return;
        }
    set_results ();
}

vector<Row> Executor::run ( const string raw_query )
{

    auto       lexer       = lexer::Lexer ( raw_query );
    const auto parser      = parser::Parser ();
    const auto analyzer    = analyzer::Analyzer ();
    auto       syntax_tree = parser.parse ( lexer );
    select_t               = static_cast<parser::SelectStatementNode *> ( syntax_tree.get () );
    analyzer.analyze ( select_t );

    view_v            = new view::View ( map_view () );
    placehoders_nodes = get_placeholders ( select_t );
    auto system       = system::System ( file_callback );

    LOG_DEBUG ( "Raw Query: " << raw_query );
    LOG_DEBUG ( "Generated AST: \n" << select_t->serialize () );
    view_v->print_headers ();
    system.query ( select_t->table->value );

    return {};
}

unordered_map<string, vector<parser::PlaceholderNode *>> get_placeholders ( parser::Node *root )
{
    const auto type = root->get_type ();

    if ( dynamic_cast<parser::PlaceholderNode *> ( root ) )
        {
            auto                              node = static_cast<parser::PlaceholderNode *> ( root );
            vector<parser::PlaceholderNode *> vec  = { node };
            return { { node->identifier_name, vec } };
        }

    switch ( type )
        {
        case parser::NodeType::FUNCTION:
            {
                auto node = static_cast<parser::FunctionNode *> ( root );
                return get_placeholders ( node->value.get () );
            }

        case parser::NodeType::COLUMN:
            {
                auto node = static_cast<parser::ColumnNode *> ( root );
                return get_placeholders ( node->value.get () );
            }
        case parser::NodeType::BINARY_EXPRESSION:
            {
                auto node = static_cast<parser::BinaryExpressionNode *> ( root );

                auto left = get_placeholders ( node->left.get () );
                merge_dictionaries ( left, get_placeholders ( node->right.get () ) );
                return left;
            }
        case parser::NodeType::SELECT_STATEMENT:
            {
                auto                                                     node = static_cast<parser::SelectStatementNode *> ( root );
                unordered_map<string, vector<parser::PlaceholderNode *>> result;
                for ( const auto &column : node->columns )
                    {
                        merge_dictionaries ( result, get_placeholders ( column.get () ) );
                    }
                if ( node->filter != nullptr )
                    {
                        merge_dictionaries ( result, get_placeholders ( node->filter.get () ) );
                    }
                return result;
            }
        case parser::NodeType::UNARY_EXPRESSION:
            {
                auto node = dynamic_cast<parser::UnaryExpressionNode *> ( root );
                return get_placeholders ( node->value.get () );
            }
        default:
            break;
        }
    return {};
}

void merge_dictionaries ( unordered_map<string, vector<parser::PlaceholderNode *>> &left, unordered_map<string, vector<parser::PlaceholderNode *>> right )
{
    if ( left.empty () )
        {
            left = right;
            return;
        }
    if ( right.empty () )
        {
            return;
        }

    for ( auto &r : right )
        {
            if ( left.find ( r.first ) != left.end () )
                {
                    // just combine the values
                    auto &values = left.find ( r.first )->second;
                    values.insert ( values.end (), r.second.begin (), r.second.end () );
                    continue;
                }
            left.insert ( r );
        }
}
void append_data_to_placeholders ( const system::FileInformation &file, unordered_map<string, vector<parser::PlaceholderNode *>> &nodes )
{
    for ( auto &r : nodes )
        {
            if ( r.first == "NAME" )
                {
                    for ( auto &placeholder : r.second )
                        {
                            placeholder->value->value = file.name;
                        }
                }
            if ( r.first == "SIZE" )
                {
                    for ( auto &placeholder : r.second )
                        {
                            placeholder->value->value = to_string ( file.size );
                        }
                }
            if ( r.first == "RELATIVE_PATH" )
                {
                    for ( auto &placeholder : r.second )
                        {
                            placeholder->value->value = file.relative_path;
                        }
                }
            if ( r.first == "CREATION" )
                {
                    for ( auto &placeholder : r.second )
                        {
                            placeholder->value->value = file.creation;
                        }
                }
        }
}

bool filter ( parser::Node *condition )
{
    if ( condition == nullptr )
        {
            return true;
        }
    const auto result = dynamic_cast<parser::ValueNode *> ( condition )->evaluate ();
    return result == "1";
}

vector<view::ColumnView> map_view ()
{
    vector<view::ColumnView> columns;
    columns.reserve ( select_t->columns.size () );
    for ( const auto &column : select_t->columns )
        {
            columns.emplace_back ( 30, column->name );
        }
    return columns;
}

void set_results ()
{
    vector<string> results;
    results.reserve ( select_t->columns.size () );
    for ( auto &column : select_t->columns )
        {
            results.push_back ( column->evaluate () );
        }
    view_v->print_line ( results );
}