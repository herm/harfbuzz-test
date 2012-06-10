#define HAVE_FREETYPE
#include <iostream>
#include <string>
#include <fstream>

#include <boost/iostreams/device/mapped_file.hpp>

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

using namespace std;
namespace mapnik
{
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
        hb_buffer_reset(buffer_);
        hb_buffer_add_utf8(buffer_, text.c_str(), text.length(), 0, text.length());
#if 0
        hb_buffer_set_direction(buffer, hb_direction_from_string (direction, -1));
        hb_buffer_set_script(buffer, hb_script_from_string (script, -1));
        hb_buffer_set_language(buffer, hb_language_from_string (language, -1));
#endif
        hb_bool_t result = hb_shape_full(font_, buffer_, 0 /*features*/, 0 /*num_features*/, 0 /*shapers*/);
        if (!result) {
            std::cerr << "ERROR: Shaping failed!\n";
            return; //TODO: Exception
        }
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
    mapnik::font_info unifont("/home/r2d2/build/mapnik/fonts/unifont-5.1.20080907.ttf");
    std::cout << "Hello World!\n";
    unifont.process_text("Hello World!");
    std::cout << "Complex text:\n";
    unifont.process_text("कि கே કિ");
    return 0;
}

