#ifndef PV_GLMESH_H
#define PV_GLMESH_H

#include <vector>

//TODO: We don't need this to be in a separate header
//since only the render needs to know about it.

struct Vertex;

class GLMesh{
public:
    GLMesh(void);
	GLMesh(const Vertex* vertices, int n_vertices);
	~GLMesh(void);
	
    void set(const Vertex* vertices, int n_vertices);
	void draw(void)const;
	void draw_instanced(int n_instances)const;

private:
    unsigned int n_vertices_;
	unsigned int vbo_;
};
#endif
