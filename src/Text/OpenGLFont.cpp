#include "include/Text/OpenGLFont.h"
#include <GL/glew.h>
#include <cstdio>
#include <cmath>
#define STB_TRUETYPE_IMPLEMENTATION
#include "include/Text/stb_truetype.h"

Glyph::Glyph(const stbtt_fontinfo* info, uint32_t character):
    font_info_(info), bitmap_(nullptr), tex_(0)
{
    float scale = stbtt_ScaleForMappingEmToPixels(font_info_, 48);
    //Bitmap
    int xoff, yoff;
    bitmap_ = stbtt_GetCodepointBitmap(font_info_, scale, scale, character, &width_, &height_, &xoff, &yoff);

    //Texture
    glGenTextures(1, &tex_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width_, height_, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //Metrics
    int advance_width, left_bearing;
    stbtt_GetCodepointHMetrics(font_info_, character, &advance_width, &left_bearing);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(font_info_, &ascent, &descent, &lineGap);
    advance_height_ = ascent - descent + lineGap;

    advance_height_ = advance_height_ * scale;
    advance_width_ = advance_width * scale;
    left_bearing_ = left_bearing * scale;

    int x0, y0, x1, y1;
    if(stbtt_GetCodepointBox(font_info_, character, &x0, &y0, &x1, &y1)){
        top_bearing_ = y1 * scale;
        printf("%f\n", top_bearing_);
    }
}

Glyph::~Glyph(void){
    stbtt_FreeBitmap(bitmap_, nullptr);
}


OpenGLFont::OpenGLFont(void):
	defaultWidth_(48)
{}

//Consider creating an additional function that returns multiple characters per font
const Glyph* OpenGLFont::get_char_glyph(std::string fontName, uint32_t character){
	auto font_itr = FontMap_.find(fontName);
	//Face doesn't exist yet. We have to create it
	if(font_itr == FontMap_.end()){
        int error = 1;
        unsigned char* ttf_data = nullptr;
        stbtt_fontinfo info;

        FILE* fp = fopen(fontName.c_str(), "rb");
        if(!fp) error = 0;
        else{
            fseek(fp, 0L, SEEK_END);
            auto sz = ftell(fp);
            fseek(fp, 0L, SEEK_SET);
            ttf_data = new unsigned char[sz];
            fread(ttf_data, sz, 1, fp);
            error = stbtt_InitFont(&info, ttf_data, 0);
        }

		if(error == 0){
			printf("Couldn't load font %s.\n", fontName.c_str());
			return nullptr;
		}
		//FT_Set_Pixel_Sizes(font.face_, 0, defaultWidth_); //Pre-set width
        auto pair = FontMap_.emplace(fontName, info);
        if(!pair.second) return nullptr; //TODO: Handle error.
        font_itr = pair.first;
	}

    Font& font = font_itr->second;

	auto glyph_itr = font.charmap_.find(character);
	//Glyph was not previously loaded. We need to do this now
	if(glyph_itr == font.charmap_.end()){
        auto pair = font.charmap_.emplace(std::piecewise_construct, std::make_tuple(character), std::make_tuple(&font.info_, character));
        if(!pair.second) return nullptr; //TODO: Handle error.
        glyph_itr = pair.first;
	}

	return &glyph_itr->second;
}

