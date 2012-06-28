#include "text_layout.hpp"
#include "text_shaping.hpp"
#include <iostream>
#include <string>
#include <list>


int main()
{
    mapnik::text_layout layout(0.0, 0.0);
    mapnik::char_properties dummy;
    layout.add_text("Hello World", dummy);
    layout.add_text("किகே", dummy);
    layout.add_text("وگرىmixed", dummy);
    layout.add_text("وگرى", dummy);
    layout.add_text("German umlauts: äöüß", dummy);
    layout.add_text("äöüß", dummy);
    layout.add_text("Ligatures: ff fi ffi fl", dummy);
    layout.shape_text();

    return 0;
}

