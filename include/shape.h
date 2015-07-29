#ifndef PV_SHAPE_H
#define PV_SHAPE_H

#include "vertex.h"

struct Shape{
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
