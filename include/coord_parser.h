#ifndef PV_COORD_PARSER_H
#define PV_COORD_PARSER_H

#include <glm/glm.hpp>
#include "shape.h"

//TODO: Move these structures to separate header.

//TODO: Might need recosidering
struct Particle{
    int shape_id;
    glm::vec4 rot;
    glm::vec3 pos;
    float size;
};

//TODO: Destructor/Constructor!!
struct SimConfig{
    int n_part, n_shapes;
    Particle* particles;
    Shape* shapes;
    //TODO: Should we consider spherical boxes differently?
    glm::mat3 box;
};

SimConfig parse_config(const char* file_path);

#endif
