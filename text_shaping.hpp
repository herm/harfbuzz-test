#ifndef MAPNIK_TEXT_SHAPING_HPP
#define MAPNIK_TEXT_SHAPING_HPP



//ICU
#include <unicode/unistr.h>
class hb_font_t;
class hb_buffer_t;

namespace mapnik
{

class text_shaping
{
public:
    //TODO: Get font file from font name
    text_shaping();
    ~text_shaping();

    void process_text(UnicodeString const& text);

protected:
    static void free_data(void *data);

    void load_font();

    hb_font_t *font_;
    hb_buffer_t *buffer_;
};
} //ns mapnik

#endif // TEXT_SHAPING_HPP
