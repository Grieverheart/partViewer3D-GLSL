#ifndef PV_COORD_PARSER_H
#define PV_COORD_PARSER_H

#include <glm/glm.hpp>
#include "shape.h"
#include "particle.h"

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
