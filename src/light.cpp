#include "../include/light.h"

CLight::CLight(void){
	m_DirectionLocation = -1;
}

CLight::~CLight(void){
}

CLight::CLight(glm::vec3 position, glm::vec3 direction){
	m_position = position;
	m_direction = direction;
	m_SpecularIntensity = 0.5f;
	m_DiffuseIntensity = 0.5f;
	m_AmbientIntensity = 1.2f;
	m_Intensity = 0.6f;
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

glm::vec3 CLight::getPosition(void){
	return m_position;
}

glm::vec3 CLight::getDirection(void){
	return m_direction;
}

void CLight::uploadDirection(glm::mat4 ViewMatrix){
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
