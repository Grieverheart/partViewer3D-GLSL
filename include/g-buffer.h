#ifndef __GBUFF_H
#define __GBUFF_H

#include "main.h"

class CGBuffer{
public:
	enum GBUFF_TEXTURE_TYPE{
		GBUFF_TEXTURE_TYPE_DIFFUSE,
		GBUFF_TEXTURE_TYPE_NORMAL,
		GBUFF_NUM_TEXTURES
	};
	CGBuffer(void);
	~CGBuffer(void);
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
	void BindForWriting(void);
	void BindForReading(void);
	void BindForSSAO(void);
	void SetReadBuffer(GBUFF_TEXTURE_TYPE TextureType);
private:
	GLuint m_fbo;
	GLuint m_textures[GBUFF_NUM_TEXTURES];
	GLuint m_depthTexture;
};

#endif