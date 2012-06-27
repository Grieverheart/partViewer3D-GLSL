#ifndef __TEXTURE_H
#define __TEXTURE_H

#include "main.h"
#include "bitmap.h"

class Texture{
public:
	~Texture(void);
	Texture(GLenum textureTarget, const char *filename);
	bool Load(void);
	void Bind(GLenum textureUnit);
private:
	Bitmap *image;
    const char *filename;
    GLenum textureTarget;
    GLuint textureObj;
};

#endif