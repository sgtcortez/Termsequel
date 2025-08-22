#ifndef __TERMSEQUEL_EXECUTOR__
#define __TERMSEQUEL_EXECUTOR__

#include <string>
#include <vector>

namespace termsequel {
namespace executor {

struct Row
{
    std::string name;
    std::string value;
};

struct Executor
{
    std::vector<Row> run ( const std::string raw_query );
};
}; // namespace executor
}; // namespace termsequel

#endif