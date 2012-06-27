#ifndef __BUTTON_H
#define __BUTTON_H
#include "main.h"

class CButton{
public:
	CButton(float width, float height, glm::vec2 position, std::string type);
	~CButton(void);
	void Draw(void);
	bool isClicked(float x, float y);
	void Resize(float width_fact, float height_fact);
	
	std::string m_type;
	
private:
	float m_width;
	float m_height;
	glm::vec2 m_position;
	
	GLuint m_vbo;
	GLuint m_vao;
};

#endif