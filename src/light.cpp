#include "../include/light.h"

CLight::CLight(void){
	m_DirectionLocation = -1;
}

CLight::~CLight(void){
}

CLight::CLight(glm::vec3 position, glm::vec3 direction){
	m_position = position;
	m_direction = direction;
}

bool CLight::Init(GLuint shader_id){
	m_DirectionLocation = glGetUniformLocation(shader_id, "light.direction");
	
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
}