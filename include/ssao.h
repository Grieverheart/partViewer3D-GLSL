#ifndef __SSAO_H
#define __SSAO_H

#include "main.h"

class Cssao{
public:
	Cssao(void);
	~Cssao(void);
	
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight, unsigned int shader_id);
	void BindNoise(void);
	void UploadUniforms(void);
	void BindForWriting(void);
	void BindForReading(void);
	
private:
	void CreateKernel(void);
	void CreateNoise(void);
	
	unsigned int m_kernel_size;
	unsigned int m_noise_size;
	unsigned int m_noise_scale;
	unsigned int m_shader_id;
	GLuint m_noise_texture;
	GLuint m_ssaoTexture;
	GLuint m_fbo;
	float m_RADIUS;
	
	int m_noiseSamplerLocation;
	int m_kernLocation;
	int m_nScale_kSizeLocation;
	int m_RADIUSLocation;
	
	glm::ivec3 m_nScale_kSize;
	glm::vec3 *m_kernel;
	glm::vec3 *m_noise;
};

#endif