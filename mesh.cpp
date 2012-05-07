#include "mesh.h"

CMesh::CMesh(void){
	diffcolor = glm::vec3(0.1334, 0.545, 0.1334);
}

CMesh::~CMesh(void){
	if(vaoID != 0) glDeleteBuffers(1, &vaoID);
	if(vboID[0] != 0) glDeleteBuffers(2, &vboID[0]);
}

void CMesh::data(std::vector<glm::vec3> vertices, std::vector<glm::vec3> vertexnormals, std::vector<GLushort> elements){
	this->vertices = vertices;
	this->vertexnormals = vertexnormals;
	this->elements = elements;
}

void CMesh::upload(GLuint shaderID){
	
	glGenVertexArrays(1,&vaoID);
	glBindVertexArray(vaoID);
	
	glGenBuffers(2,&vboID[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0,3,GL_FLOAT,GL_FALSE,0,0);
	glEnableVertexAttribArray((GLuint)0);
	
	glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
	glBufferData(GL_ARRAY_BUFFER, vertexnormals.size() * sizeof(vertexnormals[0]), &vertexnormals[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1,3,GL_FLOAT,GL_FALSE,0,0);
	glEnableVertexAttribArray((GLuint)1);
	
	glGenBuffers(1,&iboID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(elements[0]), &elements[0], GL_STATIC_DRAW);
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER,0);
	
	diffcolorLocation = glGetUniformLocation(shaderID,"diffColor");
}

void CMesh::draw(void){
	glUniform3fv(diffcolorLocation, 1, &diffcolor[0]);
	GLuint size = elements.size();
	glBindVertexArray(vaoID); 
	glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_SHORT, (GLvoid*)0);
	glBindVertexArray(0);
}