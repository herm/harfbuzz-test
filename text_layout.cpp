#include "text_layout.hpp"
#include "text_shaping.hpp"

//stl
#include <iostream>

// harf-buzz
#include <harfbuzz/hb.h>

namespace mapnik
{
text_layout::text_layout(double text_ratio, double wrap_width) : text_ratio_(text_ratio), wrap_width_(wrap_width)
{
}

void text_layout::break_lines()
{
}

void text_layout::shape_text()
{
    std::list<text_item> const& list = itemizer.itemize();
    std::list<text_item>::const_iterator itr = list.begin(), end = list.end();
    for (;itr!=end; itr++)
    {
        std::string s;
        text_shaping shaper;
        shaper.process_text(itr->str);
        hb_buffer_t *buffer = shaper.get_buffer();

        unsigned num_glyphs = hb_buffer_get_length(buffer);

        hb_glyph_info_t *glyphs = hb_buffer_get_glyph_infos(buffer, NULL);
        hb_glyph_position_t *positions = hb_buffer_get_glyph_positions(buffer, NULL);
        std::cout << "Processing item '" << itr->str.toUTF8String(s) << "' (" << uscript_getName(itr->script) << "," << itr->str.length() << "," << num_glyphs << ")\n";

        for (unsigned i=0; i<num_glyphs; i++)
        {
            std::cout << "glyph codepoint:" << glyphs[i].codepoint <<
                         " cluster: " << glyphs[i].cluster <<
                         " x_advance: "<< positions[i].x_advance << "\n";
        }
    }
}

} //ns mapnik
