#ifndef __FONT_MANAGER_H
#define __FONT_MANAGER_H

#include <map>
#include <cstdint>
#include <string>
#include "stb_truetype.h"

//TODO: Add kerning.
//TODO: Subpixel.

namespace Text{

struct Glyph{
public:
    Glyph(const stbtt_fontinfo* info, uint32_t character, float scale);
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
    unsigned char* bitmap_;
    unsigned int tex_;
    int width_, height_;

    float advance_width_, left_bearing_;
    float top_bearing_;
};

struct Font{
public:
    Font(const stbtt_fontinfo& info, float default_size);
    ~Font(void);
    const Glyph* get_char_glyph(uint32_t character);
    float kern_advance(uint32_t ch1, uint32_t ch2)const;
    float line_advance(void)const{
        return advance_height_;
    }

private:
    using CharacterGlyphMap = std::map<uint32_t, Glyph>;
    float scale_;
    float advance_height_;
    stbtt_fontinfo info_;
    CharacterGlyphMap charmap_;
};

class FontManager{
public:
    FontManager(void);
    Font* get_font(std::string font_name);
    float get_default_size(void)const{
        return default_size_;
    }

private:
    float default_size_;
    std::map<std::string, Font> FontMap_;
};

};

#endif
