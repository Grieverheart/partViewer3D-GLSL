#include "../include/button.h"

CButton::CButton(float width, float height, glm::vec2 position, std::string type){
	m_width = width;
	m_height = height;
	m_position = position;
	m_type = type;
	
	float vertices[20] = {
		0.0f, 		0.0f, 		0.0f,	 1.0f, 1.0f,
		m_width,	0.0f, 		0.0f,	 0.0f, 1.0f,
		0.0f,		m_height, 	0.0f,	 1.0f, 0.0f,
		m_width,	m_height, 	0.0f,	 0.0f, 0.0f
	};
	
	for(unsigned int i = 0; i < 4; i++){
		vertices[5 * i] += position[0];
		vertices[5 * i + 1] += position[1];
	}
	
	// Create the VAO & VBO
	glGenVertexArrays(1,&m_vao);
	glBindVertexArray(m_vao);
	
	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray((GLuint)0);
	glEnableVertexAttribArray((GLuint)1);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)0);
	glVertexAttribPointer((GLuint)1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));
	
	glBindVertexArray(0);
}

CButton::~CButton(void){
	// glDeleteVertexArrays(1, &m_vao);
}

bool CButton::isClicked(float x, float y){
	float n_x = x - m_position.x;
	float n_y = y - m_position.y;
	if(n_x < m_width && n_x > 0.0 && n_y < m_height && n_y > 0.0 ) return true;
	else return false;
}

void CButton::Resize(float width_fact, float height_fact){
	float fact;
	fact = height_fact;
	width_fact /= fact;
	height_fact /= fact;
	m_width /= width_fact;
	m_height /= height_fact;
	
	m_position.x = (m_position.x + 1.0) / width_fact - 1.0;
	m_position.y = (m_position.y - 1.0) / height_fact + 1.0;
	
	float vertices[20] = {
		0.0f, 		0.0f, 		0.0f,	 1.0f, 1.0f,
		m_width,	0.0f, 		0.0f,	 0.0f, 1.0f,
		0.0f,		m_height, 	0.0f,	 1.0f, 0.0f,
		m_width,	m_height, 	0.0f,	 0.0f, 0.0f
	};
	
	for(unsigned int i = 0; i < 4; i++){
		vertices[5 * i] += m_position[0];
		vertices[5 * i + 1] += m_position[1];
	}
	
	glBindVertexArray(m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 20 * sizeof(float), &vertices[0]);
	glBindVertexArray(0);
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