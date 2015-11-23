#ifndef __SSAO_H
#define __SSAO_H

#include <glm/glm.hpp>
#include <random>

class Shader;

class Cssao{
public:
    enum SSAO_TEXTURE_TYPE{
        TEXTURE_TYPE_SSAO,
        TEXTURE_TYPE_NOISE,
        NUM_TEXTURES
    };
	Cssao(void);
	~Cssao(void);
	
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
	void UploadUniforms(const Shader& shader)const;
	void UpdateUniforms(const Shader& shader);
	void Bind(void)const;
	void UnBind(void)const;
	void BindTexture(SSAO_TEXTURE_TYPE type, int attachment_point)const;
	void Resize(unsigned int WindowWidth, unsigned int WindowHeight, Shader const *shader);
	
	float getRadius(void)const;
	void setRadius(float radius);
	unsigned int getSamples(void)const;
	void setSamples(unsigned int num_samples);
	
private:
	void CreateKernel(void);
	void CreateNoise(void);
	
	unsigned int m_kernel_size;
	unsigned int m_noise_size;

	unsigned int m_noise_texture;
	unsigned int m_ssaoTexture;
	unsigned int m_fbo;

	float m_RADIUS;

	bool m_update;
	bool m_update_kernel;
	
	glm::ivec2 m_noiseScale;
	glm::vec3 *m_kernel;
	glm::vec3 *m_noise;

    std::mt19937 rng_;
};

#endif
