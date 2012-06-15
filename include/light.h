#ifndef __LIGHT_H
#define __LIGHT_H

#include "main.h"

class CLight{
public:
	CLight(void);
	CLight(glm::vec3 position, glm::vec3 direction);
	~CLight(void);
	
	bool Init(GLuint shade_id);
	void uploadDirection(glm::mat4 ViewMatrix);
	void uploadMVP(glm::mat4 lightMVP);
	
	glm::vec3 getPosition(void);
	glm::vec3 getDirection(void);
	
private:
	glm::vec3 m_position;
	glm::vec3 m_direction;
	int m_DirectionLocation;
	int m_MVPLocation;
};

#endif