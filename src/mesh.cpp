#include "../include/mesh.h"

CMesh::CMesh(GLfloat scale){
	// diffcolor = glm::vec3(0.1334, 0.545, 0.1334);
	diffcolor = glm::vec3(0.282, 0.239, 0.545);
	this->scale = scale;
}

CMesh::~CMesh(void){
	if(vaoID != 0) glDeleteBuffers(1, &vaoID);
	if(vboID != 0) glDeleteBuffers(1, &vboID);
}

void CMesh::data(std::vector<Vertex> vertices){
	this->vertices = vertices;
}

void CMesh::upload(GLuint shaderID){
	
	glGenVertexArrays(1,&vaoID);
	glBindVertexArray(vaoID);
	
	glGenBuffers(1,&vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray((GLuint)0);
	glEnableVertexAttribArray((GLuint)1);
	glEnableVertexAttribArray((GLuint)2);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	scaleLocation = glGetUniformLocation(shaderID, "scale");
	diffColorLocation = glGetUniformLocation(shaderID, "diffColor");
	
	// if(scaleLocation == -1){
		// std::cout << "Unable to bind uniform" << std::endl;
	// }
}

void CMesh::draw(void){
	glUniform1fv(scaleLocation, 1, &scale);
	glGetError();
	glUniform3fv(diffColorLocation, 1, &diffcolor[0]);
	glGetError();
	
	glBindVertexArray(vaoID); 
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glBindVertexArray(0);
}