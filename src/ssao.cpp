#include "../include/ssao.h"
#include <cstdlib>
#include <time.h>

static inline float random(float x, float y){
	float one = (float)rand() / RAND_MAX;
	float dx = y - x;
	return dx * one + x;
}

static inline float lerp(float start, float end, float perc){
	return start + perc * (end - start);
}

Cssao::Cssao(void):
	m_kernel_size(16), m_noise_size(3), m_RADIUS(5.0f),
	m_update(false), m_update_kernel(false),
    m_kernel(NULL), m_noise(NULL)
{
	srand(2);
}

Cssao::~Cssao(void){
	delete[] m_kernel;
	delete[] m_noise;
}

void Cssao::CreateKernel(void){	
	m_kernel = new glm::vec3[m_kernel_size];
	
	for(unsigned int i = 0; i < m_kernel_size; i++){ //Create a bigger kernel so we can freely change it later
		m_kernel[i] = glm::vec3(
			random(-1.0f, 1.0f),
			random(-1.0f, 1.0f),
			random(0.0f, 1.0f)
		);
		float scale = (float)(i + 1) / (float)m_kernel_size;
		scale = lerp(0.1f, 1.0f, scale * scale);
		m_kernel[i] *= scale;
	}
}

void Cssao::CreateNoise(void){
	m_noise = new glm::vec3[m_noise_size * m_noise_size];
	
	for(unsigned int i = 0; i < m_noise_size * m_noise_size; i++){
		m_noise[i] = glm::vec3(
			random(-1.0f, 1.0f),
			random(-1.0f, 1.0f),
			0.0f
		);
		m_noise[i] = glm::normalize(m_noise[i]);
	}
	
	glGenTextures(1, &m_noise_texture);
	glBindTexture(GL_TEXTURE_2D, m_noise_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_noise_size, m_noise_size, 0, GL_RGB, GL_FLOAT, &m_noise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

bool Cssao::Init(unsigned int WindowWidth, unsigned int WindowHeight){
	m_noiseScale = glm::ivec2(WindowWidth / m_noise_size, WindowHeight / m_noise_size);
	
	CreateKernel();
	CreateNoise();
	
	//Create FBO
	glGenFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
	
	glGenTextures(1, &m_ssaoTexture);
	glBindTexture(GL_TEXTURE_2D, m_ssaoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, WindowWidth, WindowHeight, 0, GL_RED, GL_FLOAT, NULL);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_ssaoTexture, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	GLenum DrawBuffers[] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, DrawBuffers);
	
	GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
	if(Status != GL_FRAMEBUFFER_COMPLETE){
		std::cout << "FB error, status 0x" << std::hex << Status << std::endl;
		return false;
	}
	
	//Restore default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	return true;
}

void Cssao::Bind(void)const{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
}

void Cssao::UnBind(void)const{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void Cssao::BindTexture(SSAO_TEXTURE_TYPE type, int attachment_point)const{
	glActiveTexture(GL_TEXTURE0 + attachment_point);
	if(type == TEXTURE_TYPE_SSAO) glBindTexture(GL_TEXTURE_2D, m_ssaoTexture);
    else glBindTexture(GL_TEXTURE_2D, m_noise_texture);
}

void Cssao::UploadUniforms(const Shader& shader)const{
	shader.setUniform("noise", 2);
	shader.setUniform("kernelSize", (int)m_kernel_size);
	shader.setUniform("noiseScale", 1, m_noiseScale);
	shader.setUniform("kernel[0]", m_kernel_size, m_kernel[0]);
	shader.setUniform("RADIUS", m_RADIUS);
}

void Cssao::UpdateUniforms(const Shader& shader){
	if(m_update_kernel){	
		shader.setUniform("kernelSize", (int)m_kernel_size);
		shader.setUniform("kernel[0]", m_kernel_size, m_kernel[0]);
	}
	if(m_update)shader.setUniform("RADIUS", m_RADIUS);
	m_update_kernel = false;
	m_update = false;
}

void Cssao::Resize(unsigned int WindowWidth, unsigned int WindowHeight, Shader const *shader){
	glBindTexture(GL_TEXTURE_2D, m_ssaoTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, WindowWidth, WindowHeight, 0, GL_RED, GL_FLOAT, NULL);
	m_noiseScale = glm::ivec2(WindowWidth / m_noise_size, WindowHeight / m_noise_size);
	shader->setUniform("noiseScale", 1, m_noiseScale);
}


float Cssao::getRadius(void)const{
	return m_RADIUS;
}

void Cssao::setRadius(float radius){
	m_RADIUS = radius;
	m_update = true;
}

unsigned int Cssao::getSamples(void)const{
	return m_kernel_size;
}

void Cssao::setSamples(unsigned int num_samples){
	m_kernel_size = num_samples;
	delete[] m_kernel;
	CreateKernel();
	m_update_kernel = true;
}
