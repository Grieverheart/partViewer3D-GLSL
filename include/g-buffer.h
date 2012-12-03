#ifndef __GBUFF_H
#define __GBUFF_H

#include "main.h"

class CGBuffer{
public:
	enum GBUFF_TEXTURE_TYPE{
		GBUFF_TEXTURE_TYPE_DIFFUSE,
		GBUFF_TEXTURE_TYPE_NORMAL,
		// GBUFF_TEXTURE_TYPE_ID,
		GBUFF_NUM_TEXTURES
	};
	CGBuffer(void);
	~CGBuffer(void);
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
	void BindForWriting(void)const;
	void BindForReading(void)const;
	void BindForPicking(void)const;
	void BindForSSAO(void)const;
	void SetReadBuffer(GBUFF_TEXTURE_TYPE TextureType)const;
	void Resize(unsigned int WindowWidth, unsigned int WindowHeight)const;
private:
	GLuint m_fbo;
	GLuint m_textures[GBUFF_NUM_TEXTURES];
	GLuint m_depthTexture;
};

#endif