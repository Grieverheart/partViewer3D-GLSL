#ifndef __SSAO_H
#define __SSAO_H

#include "main.h"
#include <AntTweakBar.h>

class Cssao{
public:
	Cssao(void);
	~Cssao(void);
	
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight, unsigned int shader_id);
	void BindNoise(void);
	void UploadUniforms(void);
	void UpdateUniforms(void);
	void BindForWriting(void);
	void BindForReading(void);
	void Resize(unsigned int WindowWidth, unsigned int WindowHeight);
	
	float getRadius(void)const;
	void setRadius(float radius);
	unsigned int getSamples(void)const;
	void setSamples(unsigned int num_samples);
	
	
	static void TW_CALL SetRadiusCallback(const void *value, void *clientData){
		static_cast<Cssao *>(clientData)->setRadius(*static_cast<const float *>(value));
	}
	
	static void TW_CALL GetRadiusCallback(void *value, void *clientData){
		*static_cast<float *>(value) = static_cast<const Cssao *>(clientData)->getRadius();
	}
	
	static void TW_CALL SetSamplesCallback(const void *value, void *clientData){
		static_cast<Cssao *>(clientData)->setSamples(*static_cast<const unsigned int *>(value));
	}
	
	static void TW_CALL GetSamplesCallback(void *value, void *clientData){
		*static_cast<unsigned int *>(value) = static_cast<const Cssao *>(clientData)->getSamples();
	}
	
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
	bool m_update;
	bool m_update_kernel;
	
	int m_noiseSamplerLocation;
	int m_kernLocation;
	int m_noiseScaleLocation;
	int m_kernelSizeLocation;
	int m_RADIUSLocation;
	
	glm::ivec2 m_noiseScale;
	glm::vec3 *m_kernel;
	glm::vec3 *m_noise;
};

#endif