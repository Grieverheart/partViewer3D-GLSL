#include "../include/button.h"

CButton::CButton(float width, float height, glm::vec2 position, glm::vec4 color){
	m_width = width;
	m_height = height;
	m_position = position;
	m_color = color;
	
	float vertices[28] = {
		0.0f, 		0.0f, 		0.0f,	 m_color[0], m_color[1], m_color[2], m_color[3],
		m_width,	0.0f, 		0.0f,	 m_color[0], m_color[1], m_color[2], m_color[3],
		0.0f,		m_height, 	0.0f,	 m_color[0], m_color[1], m_color[2], m_color[3],
		m_width,	m_height, 	0.0f,	 m_color[0], m_color[1], m_color[2], m_color[3]
	};
	
	for(unsigned int i = 0; i < 4; i++){
		vertices[7 * i] += position[0];
		vertices[7 * i + 1] += position[1];
	}
	
	// Create the VAO & VBO
	glGenVertexArrays(1,&m_vao);
	glBindVertexArray(m_vao);
	
	GLuint m_vbo;
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 28 * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray((GLuint)0);
	glEnableVertexAttribArray((GLuint)1);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)0);
	glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	
	glBindVertexArray(0);
}

CButton::~CButton(void){
	// glDeleteVertexArrays(1, &m_vao);
}

void CButton::Draw(void){
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glDisable(GL_BLEND);
	
	glBindVertexArray(m_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}