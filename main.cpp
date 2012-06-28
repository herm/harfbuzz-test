#include "text_itemizer.hpp"
#include "text_shaping.hpp"

#define HAVE_FREETYPE
#include <iostream>
#include <string>
#include <list>

#include <boost/iostreams/device/mapped_file.hpp>

using namespace std;
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


private:
    text_itemizer itemizer;
    double text_ratio_;
    double wrap_width_;
};

text_layout::text_layout(double text_ratio, double wrap_width) : text_ratio_(text_ratio), wrap_width_(wrap_width)
{
}

} //ns mapnik

int main()
{
    mapnik::text_itemizer itemizer;
    mapnik::char_properties dummy;
    itemizer.add_text("Hello ", dummy);
    itemizer.add_text("World", dummy);
    itemizer.add_text("किகே", dummy);
    itemizer.add_text("وگرىmixed", dummy);
    itemizer.add_text("وگرى", dummy);
    std::list<mapnik::text_item> const& list = itemizer.itemize();
    std::list<mapnik::text_item>::const_iterator itr = list.begin(), end = list.end();
    for (;itr!=end; itr++)
    {
        std::string s;
        itr->str.toUTF8String(s);
        std::cout << "Text item: text: " << s << " rtl: " << itr->rtl << "script: " << uscript_getName(itr->script) << "\n";
    }
    mapnik::text_shaping unifont;
    std::cout << "Hello World!\n";
    unifont.process_text("Hello World!");
    std::cout << "Complex text:\n";
    unifont.process_text("कि கே કિ ฐู");
    return 0;
}

