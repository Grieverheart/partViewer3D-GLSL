#include "../include/g-buffer.h"
#include <iomanip>

CGBuffer::CGBuffer(void):
    m_fbo(0)
{}

CGBuffer::~CGBuffer(void){
    glDeleteTextures(GBUFF_NUM_TEXTURES, &m_textures[0]);
    glDeleteFramebuffers(1, &m_fbo);
}

bool CGBuffer::Init(unsigned int WindowWidth, unsigned int WindowHeight){
	//Create FBO
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	
	//Create gbuffer and Depth Buffer Textures
	glGenTextures(GBUFF_NUM_TEXTURES, &m_textures[0]);
	//prepare gbuffer
	for(unsigned int i = 0; i < GBUFF_NUM_TEXTURES - 1; ++i){
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        switch(i){
            case GBUFF_TEXTURE_TYPE_DIFFUSE:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
                break;
            case GBUFF_TEXTURE_TYPE_NORMAL:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WindowWidth, WindowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
                break;
        }
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, m_textures[i], 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	
	//prepare depth buffer
	glBindTexture(GL_TEXTURE_2D, m_textures[GBUFF_TEXTURE_TYPE_DEPTH]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textures[GBUFF_TEXTURE_TYPE_DEPTH], 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
	GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glDrawBuffers(GBUFF_NUM_TEXTURES - 1, DrawBuffers);
	
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
	if(Status != GL_FRAMEBUFFER_COMPLETE){
		std::cout << "FB error, status 0x" << std::hex << Status << std::endl;
		return false;
	}
	
	//Restore default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return true;
}

void CGBuffer::Bind(void)const{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void CGBuffer::UnBind(void)const{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void CGBuffer::BindTexture(GBUFF_TEXTURE_TYPE tex_type, int attachment_point)const{
    glActiveTexture(GL_TEXTURE0 + attachment_point);
    glBindTexture(GL_TEXTURE_2D, m_textures[tex_type]);
}

void CGBuffer::Resize(unsigned int WindowWidth, unsigned int WindowHeight)const{
	for(unsigned int i = 0; i < GBUFF_NUM_TEXTURES; i++){
		glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        switch(i){
            case GBUFF_TEXTURE_TYPE_DIFFUSE:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, WindowWidth, WindowHeight, 0, GL_RGB, GL_FLOAT, NULL);
                break;
            case GBUFF_TEXTURE_TYPE_NORMAL:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WindowWidth, WindowHeight, 0, GL_RGBA, GL_FLOAT, NULL);
                break;
            case GBUFF_TEXTURE_TYPE_DEPTH:
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
                break;
        }
    }
}
