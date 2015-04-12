#ifndef PV_GLMESH_H
#define PV_GLMESH_H

#include <vector>

struct Vertex;

class GLMesh{
public:
    GLMesh(void);
	GLMesh(const std::vector<Vertex>& vertices);
	~GLMesh(void);
	
    void set(const std::vector<Vertex>& vertices);
	void draw(void)const;
	void draw_instanced(int n_instances)const;

private:
    unsigned int n_vertices_;
	unsigned int vbo_;
};
#endif
