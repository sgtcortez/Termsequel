#ifndef __TERMSEQUEL_VIEW__
#define __TERMSEQUEL_VIEW__

#include <string>
#include <vector>

namespace termsequel {
namespace view {

struct ColumnView
{
    const std::uint8_t width;
    const std::string  name;
    ColumnView ( std::uint8_t width, const std::string &name );
};

class View {
  private:
    const std::vector<ColumnView> columns;

  public:
    View ( const std::vector<ColumnView> &columns );
    void print_headers () const;
    void print_line ( const std::vector<std::string> &value ) const;
};
}; // namespace view
}; // namespace termsequel

#endif
