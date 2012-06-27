#define HAVE_FREETYPE
#include <iostream>
#include <string>
#include <fstream>
#include <list>

#include <boost/iostreams/device/mapped_file.hpp>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#include <unicode/unistr.h>
#include <unicode/uscript.h>
#include <unicode/ubidi.h>
#include "scrptrun.h"

using namespace std;
namespace mapnik
{

typedef unsigned format_t; /*TODO*/

struct text_item
{
    UnicodeString str;
    UScriptCode script;
    format_t format;
    UBiDiDirection rtl;
    text_item(UnicodeString const& str) :
        str(str), script(), format(), rtl(UBIDI_LTR)
    {

    }
};

/** This class splits text into parts which all have the same
 * - direction (LTR, RTL)
 * - format
 * - script (http://en.wikipedia.org/wiki/Scripts_in_Unicode)
 **/
class text_itemizer
{
public:
    text_itemizer();
    void add_text(UnicodeString str, format_t format);
    std::list<text_item> const& itemize();
    void clear();
private:
    template<typename T> struct run
    {
        run(T data, unsigned limit) :  limit(limit), data(data){}
        unsigned limit;
        T data;
    };
    typedef run<format_t> format_run_t;
    typedef run<UBiDiDirection> direction_run_t;
    typedef run<UScriptCode> script_run_t;
    UnicodeString text;
    std::list<format_run_t> format_runs;
    std::list<direction_run_t> direction_runs;
    std::list<script_run_t> script_runs;
    void itemize_direction();
    void itemize_script();
    void create_item_list();
    std::list<text_item> output;
};

text_itemizer::text_itemizer() : text(), format_runs(), direction_runs(), script_runs()
{

}

void text_itemizer::add_text(UnicodeString str, format_t format)
{
    text += str;
    format_runs.push_back(format_run_t(format, text.length()));
}

std::list<text_item> const& text_itemizer::itemize()
{
    // format itemiziation is done by add_text()
    itemize_direction();
    itemize_script();
    create_item_list();
    return output;
}

void text_itemizer::clear()
{
    output.clear();
    text.remove();
    format_runs.clear();
}

void text_itemizer::itemize_direction()
{
    direction_runs.clear();
    UErrorCode error = U_ZERO_ERROR;
    int32_t length = text.length();
    UBiDi *bidi = ubidi_openSized(length, 0, &error);
    ubidi_setPara(bidi, text.getBuffer(), length, UBIDI_DEFAULT_LTR, 0, &error);
    if (U_SUCCESS(error))
    {
        UBiDiDirection direction = ubidi_getDirection(bidi);
        if(direction != UBIDI_MIXED)
        {
            direction_runs.push_back(direction_run_t(direction, length));
        } else
        {
            // mixed-directional
            int32_t count = ubidi_countRuns(bidi, &error);
            if(U_SUCCESS(error))
            {
                int32_t position = 0;
                for(int i=0; i<count; i++)
                {
                    int32_t length;
                    direction = ubidi_getVisualRun(bidi, i, 0, &length);
                    position += length;
                    direction_runs.push_back(direction_run_t(direction, position));
                }
            }
        }
    } else{
        std::cout << "ERROR:" << u_errorName(error) << "\n"; //TODO: Exception
    }
    if (bidi) ubidi_close(bidi);
}

void text_itemizer::itemize_script()
{
    script_runs.clear();


    ScriptRun runs(text.getBuffer(), text.length());
    while (runs.next()) {
        script_runs.push_back(script_run_t(runs.getScriptCode(), runs.getScriptEnd()));
    }
}

void text_itemizer::create_item_list()
{
    int32_t position = 0;
    std::list<script_run_t>::const_iterator script_itr = script_runs.begin(), script_end = script_runs.end();
    std::list<direction_run_t>::const_iterator dir_itr = direction_runs.begin(), dir_end = direction_runs.end();
    std::list<format_run_t>::const_iterator format_itr = format_runs.begin(), format_end = format_runs.end();
    while (position < text.length())
    {
        unsigned next_position = min(script_itr->limit, min(dir_itr->limit, format_itr->limit));
        text_item item(text.tempSubStringBetween(position, next_position));
        item.format = format_itr->data;
        item.script = script_itr->data;
        item.rtl = dir_itr->data;
        output.push_back(item);
        if (script_itr->limit == next_position)
        {
            if (script_itr == script_end) {
                //TODO: EXCEPTION
                std::cout << "Limit error\n";
            }
            script_itr++;
        }
        if (dir_itr->limit == next_position)
        {
            if (dir_itr == dir_end) {
                //TODO: EXCEPTION
                std::cout << "Limit error\n";
            }
            dir_itr++;
        }
        if (format_itr->limit == next_position)
        {
            if (format_itr == format_end) {
                //TODO: EXCEPTION
                std::cout << "Limit error\n";
            }
            format_itr++;
        }
        position = next_position;
    }
}


class font_info
{
public:
    //TODO: Get font file from font name
    font_info(std::string const& filename)
        : filename_(filename),
          font_(0),
          buffer_ (hb_buffer_create())
    {
        load_font();
    }

    ~font_info()
    {
        hb_buffer_destroy(buffer_);
        hb_font_destroy(font_);
    }

    void process_text(std::string text)
    {
        if (!font_) return;
        std::cout << "Text:" << text << " length: "<<text.length() <<"\n";
        hb_buffer_reset(buffer_);
        hb_buffer_add_utf8(buffer_, text.c_str(), text.length(), 0, text.length());
#if 0
        hb_buffer_set_direction(buffer, hb_direction_from_string (direction, -1));
        hb_buffer_set_script(buffer, hb_script_from_string (script, -1));
        hb_buffer_set_language(buffer, hb_language_from_string (language, -1));
#endif
        hb_shape(font_, buffer_, 0 /*features*/, 0 /*num_features*/);
        int num_glyphs = hb_buffer_get_length(buffer_);
        hb_glyph_info_t *hb_glyph = hb_buffer_get_glyph_infos(buffer_, NULL);
        hb_glyph_position_t *hb_position = hb_buffer_get_glyph_positions(buffer_, NULL);
        for (int i=0; i<num_glyphs; i++)
        {
            std::cout << "glyph codepoint:" << hb_glyph[i].codepoint <<
                         " cluster: " << hb_glyph[i].cluster <<
                         " mask: " << hb_glyph[i].mask <<
                         " x_advance: "<< hb_position[i].x_advance <<
                         " x_advance: "<< hb_position[i].y_advance <<
                         " x_offset: "<< hb_position[i].x_offset <<
                         " y_offset: "<< hb_position[i].y_offset << "\n";
//            std::cout << "glyph:" << hb_glyph->codepoint << "\n";
        }
    }

protected:
    static void free_data(void *data)
    {
        std::cout << "free_data()\n";
        char *tmp = (char *)data;
        delete [] tmp;
    }

    void load_font()
    {
        if (font_) return;

        char *font_data;
        unsigned int size;

        //TODO: Provide memory-mapped access

        ifstream file(filename_.c_str(), ios::in|ios::binary|ios::ate);
        if (file.is_open())
        {
            size = file.tellg();
            font_data = new char[size];
            file.seekg(0, ios::beg);
            file.read(font_data, size);
            file.close();
        } else {
            std::cerr << "Could not open font!\n";
            return ;//TODO: Raise exception
        }


        hb_blob_t *blob = hb_blob_create(font_data, size, HB_MEMORY_MODE_WRITABLE, font_data, &free_data);
        hb_face_t *face = hb_face_create(blob, 0 /*face_index*/);
        hb_blob_destroy(blob);
        font_ = hb_font_create(face);
#if 1
        //TODO: Font size
        unsigned int upem = hb_face_get_upem(face);
        hb_font_set_scale(font_, upem, upem);
#endif
        hb_face_destroy(face);
        hb_ft_font_set_funcs(font_);
    }

    std::string filename_;
    hb_font_t *font_;
    hb_buffer_t *buffer_;
};

} //ns mapnik

int main()
{
    mapnik::text_itemizer itemizer;
    itemizer.add_text("Hello ", 1);
    itemizer.add_text("World", 2);
    itemizer.add_text("किகே", 1);
    itemizer.add_text("وگرىmixed", 1);
    itemizer.add_text("وگرى", 1);
    std::list<mapnik::text_item> const& list = itemizer.itemize();
    std::list<mapnik::text_item>::const_iterator itr = list.begin(), end = list.end();
    for (;itr!=end; itr++)
    {
        std::string s;
        itr->str.toUTF8String(s);
        std::cout << "Text item: text: " << s << " rtl: " << itr->rtl << "format: " << itr->format << "script: " << uscript_getName(itr->script) << "\n";
    }
    mapnik::font_info unifont("./unifont-5.1.20080907.ttf");
    std::cout << "Hello World!\n";
    unifont.process_text("Hello World!");
    std::cout << "Complex text:\n";
    unifont.process_text("कि கே કિ ฐู");
    return 0;
}

