#include "../include/texture.h"

Texture::~Texture(void){
	if(image) delete image;
    if(textureObj != 0){
		glDeleteTextures(1, &textureObj);
    }
}

Texture::Texture(GLenum textureTarget, const char *filename){
    this->filename = filename;
    this->textureTarget = textureTarget;
	image = NULL;
}

bool Texture::Load(void){
	image = new Bitmap();
	if(image->loadBMP(filename)){
		glGenTextures(1, &textureObj);
		glBindTexture(textureTarget, textureObj);
		glTexImage2D(textureTarget, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->pixelData);
		glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glGenerateMipmap(textureTarget);
		delete image;
		return true;
	}
	else return false;
}

void Texture::Bind(GLenum textureUnit){
	glActiveTexture(textureUnit);
	glBindTexture(textureTarget, textureObj);
}