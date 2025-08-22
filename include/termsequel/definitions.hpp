#ifndef TERMSEQUEL_DEFINITIONS_H
#define TERMSEQUEL_DEFINITIONS_H

#include <cstdint>
#include <ostream>
#include <string>

namespace termsequel {
namespace definitions {
enum class ColumnDataType : std::uint8_t
{
    INVALID = 0,
    NUMERIC = 1,
    STRING  = 2,
    DATE    = 3
};

struct Column
{
    const ColumnDataType type;
    const std::string    name;

    Column ( const ColumnDataType type, const std::string name );

    bool                 operator== ( const Column &other ) const;
    friend std::ostream &operator<< ( std::ostream &os, const Column &column )
    {
        const char *values[] = { "INVALID", "NUMERIC", "STRING", "DATE" };
        os << "Column(" << column.name << " / " << values[ static_cast<std::uint8_t> ( column.type ) ] << ")";
        return os;
    }

    static const Column INVALID;
    static const Column NAME;
    static const Column RELATIVE_PATH;
    static const Column SIZE;
    static const Column OWNER_ID;
    static const Column CREATION;

    static const Column &get ( const std::string name );
};
};     // namespace definitions
};     // namespace termsequel
#endif // TERMSEQUEL_DEFINITIONS_H