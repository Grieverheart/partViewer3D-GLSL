#include "../include/shadowmap_FBO.h"
#include <iomanip>

CShadowMapFBO::CShadowMapFBO(void){
    m_fbo = 0;
    m_shadowMap = 0;
}

CShadowMapFBO::~CShadowMapFBO(void){
    if(m_fbo != 0){
		glDeleteFramebuffers(1, &m_fbo);
    }
    if(m_shadowMap != 0){
		glDeleteTextures(1, &m_shadowMap);
    }
}

bool CShadowMapFBO::Init(unsigned int WindowWidth, unsigned int WindowHeight){
	//Create FBO
	glGenFramebuffers(1, &m_fbo);
	
	//Create Depth Buffer Texture
	glGenTextures(1, &m_shadowMap);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glm::vec4 borderColor = glm::vec4(1.0,glm::vec3(0.0));
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);
	
	//Attach the depth texture to the framebuffer
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
	
	//Don't draw to the color buffer
	glDrawBuffer(GL_NONE);
	
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
	if(Status != GL_FRAMEBUFFER_COMPLETE){
		std::cout << "FB error, status 0x" << std::hex << Status << std::endl;
		return false;
	}
	
	return true;
}

void CShadowMapFBO::BindForWriting(void){
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void CShadowMapFBO::BindForReading(GLenum TextureUnit){
	glActiveTexture(TextureUnit);
	glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}