#ifndef MAPNIK_TEXT_LAYOUT_HPP
#define MAPNIK_TEXT_LAYOUT_HPP

#include "text_itemizer.hpp"

namespace mapnik
{
class text_layout
{
public:
    text_layout(double text_ratio, double wrap_width);
    inline void add_text(UnicodeString const& str, char_properties const& format)
    {
        itemizer.add_text(str, format);
    }

    void break_lines();
    void shape_text();


private:
    text_itemizer itemizer;
    double text_ratio_;
    double wrap_width_;
};
}

#endif // TEXT_LAYOUT_HPP
