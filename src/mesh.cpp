#include "include/mesh.h"
#include "include/vertex.h"
#include <GL/glew.h>

GLMesh::GLMesh(void):
    n_vertices_(0),
    vbo_(0)
{}

GLMesh::GLMesh(const Vertex* vertices, int n_vertices):
    n_vertices_(n_vertices),
    vbo_(0)
{
	glGenBuffers(1, &vbo_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, n_vertices_ * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
}

GLMesh::~GLMesh(void){
	glDeleteBuffers(1, &vbo_);
}

void GLMesh::set(const Vertex* vertices, int n_vertices){
	if(n_vertices_ == 0){
        glGenBuffers(1, &vbo_);
        n_vertices_ = n_vertices;
    }
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(GL_ARRAY_BUFFER, n_vertices_ * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
}

void GLMesh::draw(void)const{
    glBindVertexBuffer(0, vbo_, 0, sizeof(Vertex));
	glDrawArrays(GL_TRIANGLES, 0, n_vertices_);
}

void GLMesh::draw_instanced(int n_instances)const{
    glBindVertexBuffer(0, vbo_, 0, sizeof(Vertex));
	glDrawArraysInstanced(GL_TRIANGLES, 0, n_vertices_, n_instances);
}

