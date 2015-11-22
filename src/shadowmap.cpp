#include "include/shadowmap.h"
#define GL_GLEXT_PROTOTYPES
#include <GL/glcorearb.h>
#include <cstdio>

//TODO: Try variance shadow maps (VSM)

CShadowmap::CShadowmap(void):
    m_fbo(0)
{}

CShadowmap::~CShadowmap(void){
    glDeleteTextures(1, &m_texture);
    glDeleteFramebuffers(1, &m_fbo);
}

bool CShadowmap::Init(unsigned int WindowWidth, unsigned int WindowHeight){
    WindowWidth *= 2;
    WindowHeight *= 2;
	//Create FBO
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	
	//Create gbuffer and Depth Buffer Textures
	glGenTextures(1, &m_texture);
	
	//prepare depth buffer
	glBindTexture(GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, m_texture, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
	if(Status != GL_FRAMEBUFFER_COMPLETE){
		printf("FB error, status 0x%04x", Status);
		return false;
	}
	
	//Restore default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return true;
}

void CShadowmap::Bind(void)const{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void CShadowmap::BindTexture(int attachment_point)const{
    glActiveTexture(GL_TEXTURE0 + attachment_point);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

void CShadowmap::Resize(unsigned int WindowWidth, unsigned int WindowHeight)const{
    WindowWidth *= 2;
    WindowHeight *= 2;
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
}
