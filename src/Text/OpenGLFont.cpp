#include "include/Text/OpenGLFont.h"
#include <GL/glew.h>
#include <cstdio>

OpenGLFont::OpenGLFont(void):
	defaultWidth_(48)
{
	FT_Init_FreeType(&ftLibrary_);
}

OpenGLFont::~OpenGLFont(void){
}

//Consider creating an additional function that returns multiple characters per font
const OpenGLFont::Glyph* OpenGLFont::getCharGlyph(std::string fontName, uint32_t character){
	Font& font = FontMap_[fontName];
	//Face doesn't exist yet. We have to create it
	if(!font.face_){
		FT_Error error = FT_New_Face(ftLibrary_, fontName.c_str(), 0, &font.face_);
		if(error){
			printf("Couldn't load font %s. Error: 0x%.4X\n", fontName.c_str(), error);
			FontMap_.erase(fontName);
			return nullptr;
		}
		FT_Set_Pixel_Sizes(font.face_, 0, defaultWidth_); //Pre-set width
	}
	
	Glyph* glyph = &font.charMap_[character];
	//Glyph was not previously loaded. We need to do this now
	if(!glyph->tex_){
		FT_Load_Char(font.face_, character, FT_LOAD_RENDER);
		glyph->metrics_ = font.face_->glyph->metrics;
		const FT_Bitmap &bitmap = font.face_->glyph->bitmap;
		
		GLuint texobj;
		glGenTextures(1, &texobj);
		glBindTexture(GL_TEXTURE_2D, texobj);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmap.width, bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		
		glyph->tex_ = texobj;
	}
	
	return glyph;
}
