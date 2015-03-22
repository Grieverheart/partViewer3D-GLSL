#ifndef __MESH_H
#define __MESH_H

#include "main.h"
#include <vector>

struct Vertex{
	glm::vec3 _coord;
	glm::vec3 _normal;
	glm::vec2 _texCoord;
	
	Vertex(void){}
	~Vertex(void){}
	Vertex(glm::vec3 coord, glm::vec3 normal, glm::vec2 texCoord = glm::vec2(0.0, 0.0)):
		_coord(coord),
		_normal(normal),
		_texCoord(texCoord)
	{}
};

class CMesh{
public:
	CMesh(GLfloat scale);
	~CMesh(void);
	
	void upload(GLuint shaderID);
	void draw(void)const;
	void uploadInstanced(GLuint shaderID);
	void drawInstanced(unsigned int nInstances, const glm::mat4* MVPs, const glm::mat3 *NMs = NULL)const;
	void data(std::vector<Vertex> vertices);
	
private:
	
	std::vector<Vertex> vertices;
	
	GLfloat scale;
	int scaleLocation;
	
	GLuint vaoID;
	GLuint vboID[3];
};
#endif
