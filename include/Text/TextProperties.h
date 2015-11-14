#ifndef __TEXT_PROPERTIES_H
#define __TEXT_PROPERTIES_H

#include <string>

struct TextProperties{
    TextProperties(void):
        font_(""),
        width_(0),
        x_(0), y_(0)
    {}
    std::string font_;
    int width_;
    int x_, y_;
};

#endif
