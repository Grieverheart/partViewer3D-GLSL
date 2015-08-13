#ifndef __LIGHT_H
#define __LIGHT_H

#include <glm/glm.hpp>

struct CLight{
	CLight(void);
	CLight(glm::vec3 direction);
	
	glm::vec3 direction_;
	float specular_;
	float diffuse_;
	float ambient_;
	float intensity_;
};

#endif
