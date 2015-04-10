#ifndef __MESH_H
#define __MESH_H

#include <vector>
#include <glm/glm.hpp>

struct Vertex{
	glm::vec3 _coord;
	glm::vec3 _normal;
	glm::vec2 _texCoord;
	
	Vertex(glm::vec3 coord, glm::vec3 normal, glm::vec2 texCoord = glm::vec2(0.0, 0.0)):
		_coord(coord),
		_normal(normal),
		_texCoord(texCoord)
	{}
};

class CMesh{
public:
	CMesh(void);
	~CMesh(void);
	
	void upload(unsigned int shaderID);
	void draw(void)const;
	void uploadInstanced(unsigned int shaderID, int n_instances, const glm::mat4* model_array);
	void drawInstanced(void)const;
	void data(std::vector<Vertex> vertices);
	
private:
	
	std::vector<Vertex> vertices;

    int nInstances;
	
	unsigned int vaoID;
	unsigned int vboID[2];
};
#endif
