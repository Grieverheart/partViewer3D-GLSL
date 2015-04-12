#ifndef PV_MESH_H
#define PV_MESH_H

#include "vertex.h"
#include <vector>

class CMesh{
public:
	CMesh(void);
	~CMesh(void);
	
	void upload(void);
	void draw(void)const;
	void uploadInstanced(int n_instances, const glm::mat4* model_array);
	void drawInstanced(void)const;
	void data(std::vector<Vertex> vertices);
	
private:
	
	std::vector<Vertex> vertices;

    int nInstances;
	
	unsigned int vaoID;
	unsigned int vboID[2];
};
#endif
