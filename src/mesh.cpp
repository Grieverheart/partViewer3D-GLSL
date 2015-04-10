#include "include/mesh.h"
#include <GL/glew.h>

CMesh::CMesh(float scale){
	// diffcolor = glm::vec3(0.1334, 0.545, 0.1334);
	this->scale = scale;
}

CMesh::~CMesh(void){
	if(vaoID != 0) glDeleteBuffers(1, &vaoID);
	if(vboID != 0) glDeleteBuffers(2, &vboID[0]);
}

void CMesh::data(std::vector<Vertex> vertices){
	this->vertices = vertices;
}

void CMesh::upload(unsigned int shaderID){
	
	glGenVertexArrays(1,&vaoID);
	glBindVertexArray(vaoID);
	
	glGenBuffers(1,&vboID[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray((GLuint)0);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray((GLuint)1);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
	glEnableVertexAttribArray((GLuint)2);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(glm::vec3)));
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	scaleLocation = glGetUniformLocation(shaderID, "scale");
	
	// if(scaleLocation == -1){
		// std::cout << "Unable to bind uniform" << std::endl;
	// }
}

void CMesh::draw(void)const{
	//glUniform1fv(scaleLocation, 1, &scale);
	glGetError();
	
	glBindVertexArray(vaoID); 
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	glBindVertexArray(0);
}

void CMesh::uploadInstanced(GLuint shaderID, int n_instances, const glm::mat4* ModelArray){
    nInstances = n_instances;
	
	glGenVertexArrays(1,&vaoID);
	glBindVertexArray(vaoID);
	
	glGenBuffers(2,&vboID[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	
	glEnableVertexAttribArray((GLuint)0);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray((GLuint)1);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
	glEnableVertexAttribArray((GLuint)2);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(glm::vec3)));
	
	glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
	for(int i = 0; i < 4; i++){ //MVP Matrices
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (const GLvoid*)(sizeof(float) * i * 4));
		glVertexAttribDivisor(3 + i, 1);
	}
	glBufferData(GL_ARRAY_BUFFER, nInstances * sizeof(glm::mat4), ModelArray, GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	scaleLocation = glGetUniformLocation(shaderID, "scale");
	
	// if(scaleLocation == -1){
		// std::cout << "Unable to bind uniform" << std::endl;
	// }
}

void CMesh::drawInstanced(void)const{
	glUniform1fv(scaleLocation, 1, &scale);
	glBindVertexArray(vaoID); 
	glDrawArraysInstanced(GL_TRIANGLES, 0, vertices.size(), nInstances);
	glBindVertexArray(0);
}
