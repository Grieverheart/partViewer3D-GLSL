#include "include/accumulator.h"
#include <GL/glew.h>
#include <cstdio>

Accumulator::Accumulator(void):
    m_fbo(0)
{}

Accumulator::~Accumulator(void){
    glDeleteTextures(1, &m_texture);
    glDeleteFramebuffers(1, &m_fbo);
}

bool Accumulator::Init(unsigned int WindowWidth, unsigned int WindowHeight){
	//Create FBO
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	
	//Create gbuffer and Depth Buffer Textures
	glGenTextures(1, &m_texture);
	//prepare gbuffer
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + 0, GL_TEXTURE_2D, m_texture, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
	
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
	if(Status != GL_FRAMEBUFFER_COMPLETE){
		printf("FB error, status 0x%04x", Status);
		return false;
	}
	
	//Restore default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return true;
}

void Accumulator::Bind(void)const{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void Accumulator::UnBind(void)const{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Accumulator::BindTexture(int attachment_point)const{
    glActiveTexture(GL_TEXTURE0 + attachment_point);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

void Accumulator::Resize(unsigned int WindowWidth, unsigned int WindowHeight)const{
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
}
