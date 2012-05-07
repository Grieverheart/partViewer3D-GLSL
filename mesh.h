#ifndef __MESH_H
#define __MESH_H

#include "main.h"
#include <vector>

class CMesh{
public:
	CMesh(void);
	~CMesh(void);
	
	void upload(GLuint shaderID);
	void draw(void);
	void data(std::vector<glm::vec3> vertices, std::vector<glm::vec3> vertexnormals, std::vector<GLushort> elements);
	
private:
	
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> vertexnormals;
	std::vector<GLushort> elements;
	
	glm::vec3 diffcolor;
	
	int diffcolorLocation;
	
	GLuint vaoID;
	GLuint vboID[2];
	GLuint iboID;
};
#endif