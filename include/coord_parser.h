#ifndef PV_COORD_PARSER_H
#define PV_COORD_PARSER_H

#include <glm/glm.hpp>
#include <vector>
#include "vertex.h"
#include "shape.h"

//TODO: Might need recosidering
struct Particle{
    int shape_id;
    glm::vec4 rot;
    glm::vec3 pos;
    float size;
};

//TODO: Destructor/Constructor!!
struct SimConfig{
    int n_part;
    Particle* particles;
    //TODO: Should we consider spherical boxes differently?
    glm::mat3 box;

    int n_shapes;
    Shape* shapes;
};

SimConfig parse_config(const char* file_path);

#endif
