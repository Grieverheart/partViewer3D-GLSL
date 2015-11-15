#ifndef __TEXT_PROPERTIES_H
#define __TEXT_PROPERTIES_H

#include <string>

struct TextProperties{
    TextProperties(const char* font, int width, const glm::vec4& color, int x, int y):
        color_(color),
        font_(font),
        width_(width),
        x_(x), y_(y)
    {}

    TextProperties(void):
        color_(0.0),
        font_(""),
        width_(0),
        x_(0), y_(0)
    {}

    glm::vec4 color_;
    std::string font_;
    int width_;
    int x_, y_;
};

#endif
