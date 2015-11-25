#include "include/Text/FontManager.h"
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <cstdio>
#include <cmath>
#define STB_TRUETYPE_IMPLEMENTATION
#include "include/Text/stb_truetype.h"

using namespace Text;

Glyph::Glyph(const stbtt_fontinfo* info, uint32_t character, float scale):
    bitmap_(nullptr), tex_(0)
{
    //Bitmap
    int xoff, yoff;
    bitmap_ = stbtt_GetCodepointBitmap(info, scale, scale, character, &width_, &height_, &xoff, &yoff);

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
    stbtt_GetCodepointHMetrics(info, character, &advance_width, &left_bearing);

    advance_width_ = advance_width * scale;
    left_bearing_ = left_bearing * scale;

    int x0, y0, x1, y1;
    if(stbtt_GetCodepointBox(info, character, &x0, &y0, &x1, &y1)){
        top_bearing_ = y1 * scale;
        printf("%f\n", top_bearing_);
    }
}

Glyph::~Glyph(void){
    stbtt_FreeBitmap(bitmap_, nullptr);
}

Font::Font(const stbtt_fontinfo& info, float default_size):
    info_(info)
{
    scale_ = stbtt_ScaleForMappingEmToPixels(&info_, default_size);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info_, &ascent, &descent, &lineGap);
    advance_height_ = scale_ * (ascent - descent + lineGap);
}

Font::~Font(void){
    delete[] info_.data;
}


//Consider creating an additional function that returns multiple characters per font
const Glyph* Font::get_char_glyph(uint32_t character){
	auto glyph_itr = charmap_.find(character);
	//Glyph was not previously loaded. We need to do this now
	if(glyph_itr == charmap_.end()){
        auto pair = charmap_.emplace(std::piecewise_construct, std::make_tuple(character), std::make_tuple(&info_, character, scale_));
        if(!pair.second) return nullptr; //TODO: Handle error.
        glyph_itr = pair.first;
	}

	return &glyph_itr->second;
}

float Font::kern_advance(uint32_t ch1, uint32_t ch2)const{
    return stbtt_GetCodepointKernAdvance(&info_, ch1, ch2);
}

FontManager::FontManager(void):
	default_size_(48.0f)
{}

Font* FontManager::get_font(std::string font_name){
	auto font_itr = FontMap_.find(font_name);
	//Face doesn't exist yet. We have to create it
	if(font_itr == FontMap_.end()){
        int error = 1;
        unsigned char* ttf_data = nullptr;
        stbtt_fontinfo info;

        FILE* fp = fopen(font_name.c_str(), "rb");
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
			printf("Couldn't load font %s.\n", font_name.c_str());
			return nullptr;
		}
		//FT_Set_Pixel_Sizes(font.face_, 0, default_size_); //Pre-set width
        auto pair = FontMap_.emplace(std::piecewise_construct, std::make_tuple(font_name), std::make_tuple(info, default_size_));
        if(!pair.second) return nullptr; //TODO: Handle error.
        font_itr = pair.first;
	}

    return &font_itr->second;
}

