#ifndef PV_SHAPE_H
#define PV_SHAPE_H

struct Shape{
    ~Shape(void){
        if(shape_type == MESH) delete[] mesh.vertices;
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

    enum eShapeType{
        MESH, SPHERE,
        OTHER //Not yet implemented
    }shape_type;
};

#endif
