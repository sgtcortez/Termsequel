#include "termsequel/definitions.hpp"

#include <string>
#include <unordered_map>

namespace termsequel {
namespace definitions {
Column const Column::INVALID ( ColumnDataType::INVALID, "" );
Column const Column::NAME ( ColumnDataType::STRING, "NAME" );
Column const Column::RELATIVE_PATH ( ColumnDataType::STRING, "RELATIVE_PATH" );
Column const Column::SIZE ( ColumnDataType::NUMERIC, "SIZE" );
Column const Column::OWNER_ID ( ColumnDataType::NUMERIC, "OWNER_ID" );
Column const Column::CREATION ( ColumnDataType::STRING, "CREATION" );
}; // namespace definitions
}; // namespace termsequel

using namespace std;
using namespace termsequel::definitions;

const unordered_map<string, const Column &> AVAILABLE_COLUMNS = { { "NAME", Column::NAME },
                                                                  { "RELATIVE_PATH", Column::RELATIVE_PATH },
                                                                  { "SIZE", Column::SIZE },
                                                                  { "OWNER_ID", Column::OWNER_ID },
                                                                  { "CREATION", Column::CREATION } };

Column::Column ( const ColumnDataType type, const std::string name ) : type ( type ), name ( name )
{
}

bool Column::operator== ( const Column &other ) const
{
    return type == other.type && name == other.name;
}

const Column &Column::get ( const std::string name )
{
    auto result = AVAILABLE_COLUMNS.find ( name );
    if ( result == AVAILABLE_COLUMNS.end () )
        {
            return Column::INVALID;
        }
    return result->second;
}
