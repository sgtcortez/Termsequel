#include "termsequel/view/view.hpp"

#include <iomanip>
#include <iostream>

using namespace termsequel;
using namespace view;
using namespace std;

ColumnView::ColumnView ( uint8_t width, const string &name ) : width ( width ), name ( name )
{
}

View::View ( const vector<ColumnView> &columns ) : columns ( columns )
{
}

void View::print_headers () const
{
    for ( const auto &column : this->columns )
        {
            cout << left << setw ( column.width ) << column.name << " ";
        }
    cout << endl;
}

void View::print_line ( const vector<std::string> &value ) const
{
    if ( value.size () != columns.size () )
        {
            throw "Invalid!";
        }
    for ( size_t index = 0; index < value.size (); index++ )
        {
            cout << left << setw ( columns[ index ].width ) << value[ index ] << " ";
        }
    cout << endl;
}
