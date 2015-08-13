#include "include/light.h"

CLight::CLight(void):
	specular_(1.0f), diffuse_(1.0f), ambient_(0.6f), intensity_(1.0f)
{}

CLight::CLight(glm::vec3 direction):
	direction_(glm::normalize(direction)),
	specular_(1.0f), diffuse_(1.0f), ambient_(0.6f), intensity_(1.0f)
{}

