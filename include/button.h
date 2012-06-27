#ifndef __BUTTON_H
#define __BUTTON_H
#include "main.h"

class CButton{
public:
	CButton(float width, float height, glm::vec2 position, glm::vec4 color);
	~CButton(void);
	void Draw(void);
	
private:
	float m_width;
	float m_height;
	glm::vec2 m_position;
	glm::vec4 m_color;
	
	GLuint m_vao;
};

#endif