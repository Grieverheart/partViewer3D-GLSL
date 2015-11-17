#ifndef __OPENGL_FONT_H
#define __OPENGL_FONT_H

#include <map>
#include <cstdint>
#include "stb_truetype.h"

struct Glyph{
public:
    Glyph(const stbtt_fontinfo* info, uint32_t character);
    Glyph(const Glyph& other) = delete;
    ~Glyph(void);

    int width(void)const{
        return width_;
    }

    int height(void)const{
        return height_;
    }

    float advance_width(void)const{
        return advance_width_;
    }

    float advance_height(void)const{
        return advance_height_;
    }

    float left_bearing(void)const{
        return left_bearing_;
    }

    float top_bearing(void)const{
        return top_bearing_;
    }

    unsigned int tex_id(void)const{
        return tex_;
    }

private:
    const stbtt_fontinfo* font_info_;
    unsigned char* bitmap_;
    unsigned int tex_;
    int width_, height_;

    float advance_width_, left_bearing_;
    float top_bearing_;
    float advance_height_; //TODO: Move to Font
};

class OpenGLFont{
public:
    OpenGLFont(void);

    const Glyph* get_char_glyph(std::string fontName, uint32_t character);

private:
    using CharacterGlyphMap = std::map<uint32_t, Glyph>;
    struct Font{
        Font(const stbtt_fontinfo& info):
            info_(info)
        {}

        ~Font(void){
            delete[] info_.data;
        }

        stbtt_fontinfo info_;
        CharacterGlyphMap charmap_;
    };

    unsigned int defaultWidth_;
    std::map<std::string, Font> FontMap_;
};

#endif
