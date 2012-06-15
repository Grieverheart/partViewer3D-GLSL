#ifndef __SHFBO_H
#define __SHFBO_H

#include "main.h"

class CShadowMapFBO{
public:
	CShadowMapFBO(void);
	~CShadowMapFBO(void);
	bool Init(unsigned int WindowWidth, unsigned int WindowHeght);
	void BindForWriting(void);
	void BindForReading(GLenum TextureUnit);
private:
	GLuint m_fbo;
	GLuint m_shadowMap;
};

#endif