#ifndef __TERMSEQUEL_ERROR__
#define __TERMSEQUEL_ERROR__

#include <cstdint>
#include <string>

namespace termsequel {
namespace error {
enum class Type : std::uint8_t
{
    LEXICAL_ERROR  = 1,
    SYNTAX_ERROR   = 2,
    SEMANTIC_ERROR = 3,
};

class Error final {
  public:
    const Type        type;
    const std::string message;
    Error ( Type type, const std::string &message );
};
}; // namespace error
}; // namespace termsequel

#endif