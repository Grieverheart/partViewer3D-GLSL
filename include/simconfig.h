#ifndef PV_SIMCONFIG_H
#define PV_SIMCONFIG_H

#include <glm/glm.hpp>
#include <vector>
#include "shape.h"
#include "particle.h"

struct SimConfig{
    std::vector<Particle> particles;
    std::vector<Shape> shapes;
    //TODO: Should we consider spherical boxes differently?
    glm::mat3 box;
};

#endif
