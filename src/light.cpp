#include "../include/light.h"

CLight::CLight(void){
	m_DirectionLocation = -1;
}

CLight::~CLight(void){
}

CLight::CLight(glm::vec3 direction):
	m_direction(glm::normalize(direction))
{
	m_SpecularIntensity = 1.0f;
	m_DiffuseIntensity = 1.0f;
	m_AmbientIntensity = 0.6f;
	m_Intensity = 1.0f;
}

bool CLight::Init(GLuint shader_id){
	m_DirectionLocation = glGetUniformLocation(shader_id, "light.direction");
	m_SpecularIntensityLocation = glGetUniformLocation(shader_id, "light.Si");
	m_DiffuseIntensityLocation = glGetUniformLocation(shader_id, "light.Di");
	m_AmbientIntensityLocation = glGetUniformLocation(shader_id, "light.Ai");
	m_IntensityLocation = glGetUniformLocation(shader_id, "light.Intensity");
	
	if(m_DirectionLocation == -1) return false;
	return true;
}

const glm::vec3& CLight::getDirection(void)const{
	return m_direction;
}

void CLight::setDirection(const glm::vec3& dir){
	m_direction = dir;
}

void CLight::uploadDirection(glm::mat4 ViewMatrix)const{
	glm::vec3 lightViewDirection = glm::mat3(ViewMatrix) * m_direction;
	glUniform3fv(m_DirectionLocation, 1, &lightViewDirection[0]);
	glUniform1fv(m_SpecularIntensityLocation, 1, &m_SpecularIntensity);
	glUniform1fv(m_DiffuseIntensityLocation, 1, &m_DiffuseIntensity);
	glUniform1fv(m_AmbientIntensityLocation, 1, &m_AmbientIntensity);
	glUniform1fv(m_IntensityLocation, 1, &m_Intensity);
}

float CLight::getSpecInt(void)const{
	return m_SpecularIntensity;
}

void CLight::setSpecInt(float value){
	m_SpecularIntensity = value;
}

float CLight::getDiffInt(void)const{
	return m_DiffuseIntensity;
}

void CLight::setDiffInt(float value){
	m_DiffuseIntensity = value;
}

float CLight::getInt(void)const{
	return m_Intensity;
}

void CLight::setInt(float value){
	m_Intensity = value;
}

float CLight::getAmbientInt(void)const{
	return m_AmbientIntensity;
}

void CLight::setAmbientInt(float value){
	m_AmbientIntensity = value;
}
