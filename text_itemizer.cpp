//mapnik
#include "text_itemizer.hpp"
#include "scrptrun.h"

// stl
#include <iostream>
#include <algorithm>

namespace mapnik
{

text_itemizer::text_itemizer() : text(), format_runs(), direction_runs(), script_runs()
{

}

void text_itemizer::add_text(UnicodeString str, char_properties const& format)
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
        std::cerr << "ERROR:" << u_errorName(error) << "\n"; //TODO: Exception
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
        unsigned next_position = std::min(script_itr->limit, std::min(dir_itr->limit, format_itr->limit));
        text_item item(text.tempSubStringBetween(position, next_position));
        item.format = format_itr->data;
        item.script = script_itr->data;
        item.rtl = dir_itr->data;
        output.push_back(item);
        if (script_itr->limit == next_position)
        {
            if (script_itr == script_end) {
                //TODO: EXCEPTION
                std::cerr << "Limit error\n";
            }
            script_itr++;
        }
        if (dir_itr->limit == next_position)
        {
            if (dir_itr == dir_end) {
                //TODO: EXCEPTION
                std::cerr << "Limit error\n";
            }
            dir_itr++;
        }
        if (format_itr->limit == next_position)
        {
            if (format_itr == format_end) {
                //TODO: EXCEPTION
                std::cerr << "Limit error\n";
            }
            format_itr++;
        }
        position = next_position;
    }
}
} //ns mapnik
