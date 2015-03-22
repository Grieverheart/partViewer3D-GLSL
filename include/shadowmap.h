#ifndef __SHADOWMAP_H
#define __SHADOWMAP_H

#include "main.h"

class CShadowmap{
public:
	CShadowmap(void);
	~CShadowmap(void);
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
	void Bind(void)const;
    void BindTexture(int attachment_point)const;
	void Resize(unsigned int WindowWidth, unsigned int WindowHeight)const;
private:
	GLuint m_fbo;
	GLuint m_texture;
};

#endif
