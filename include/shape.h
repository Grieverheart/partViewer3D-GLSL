#ifndef PV_SHAPE_H
#define PV_SHAPE_H

#include "vertex.h"

//TODO: Perhaps add definitions for move constructors for improved performance.
struct Shape{
    Shape(void){}
    Shape(const Shape& other){
        *this = other;
    }

    Shape& operator=(const Shape& other){
        type = other.type;
        if(type == MESH){
            mesh.n_vertices = other.mesh.n_vertices;
            mesh.vertices = new Vertex[mesh.n_vertices];
            for(int i = 0; i < mesh.n_vertices; ++i){
                mesh.vertices[i] = other.mesh.vertices[i];
            }
        }
        return *this;
    }

    ~Shape(void){
        if(type == MESH) delete[] mesh.vertices;
    }

    struct Mesh{
        Vertex* vertices;
        int n_vertices;
    };

    struct Sphere{};

    union{
        Mesh mesh;
        Sphere sphere;
    };

    //int id; //TODO: We might need this in case shapes are not in order

    enum Type{
        MESH, SPHERE,
        OTHER //Not yet implemented
    }type;
};

#endif
