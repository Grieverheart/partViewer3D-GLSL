#ifndef __OPENGL_FONT_H
#define __OPENGL_FONT_H

#include <map>
#include <cstdint>
#include <ft2build.h>
#include FT_FREETYPE_H

class OpenGLFont{
public:
    struct Glyph{
        Glyph(void):
            tex_(0)
        {}
        FT_Glyph_Metrics metrics_;
        unsigned int tex_;
    };

public:
    OpenGLFont(void);
    ~OpenGLFont(void);
    const Glyph* getCharGlyph(std::string fontName, uint32_t character);
    void setDefaultWidth(unsigned int width){
        defaultWidth_ = width;
    }
    unsigned int getDefaultWidth(void)const{
        return defaultWidth_;
    }
    
private:
    unsigned int defaultWidth_;
    
    typedef std::map<uint32_t, Glyph> CharacterGlyphMap;
    struct Font{
        Font(void):
            face_(nullptr)
        {}
        FT_Face face_;
        CharacterGlyphMap charMap_;
    };
    std::map<std::string, Font> FontMap_;
    FT_Library ftLibrary_;
};

#endif
