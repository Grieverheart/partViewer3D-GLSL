#ifndef PV_SIMCONFIG_H
#define PV_SIMCONFIG_H

#include <glm/fwd.hpp>
#include "shape.h"
#include "particle.h"

struct SimConfig{
    Particle* particles;
    Shape* shapes;
    size_t n_particles, n_shapes;
    glm::mat3 box;
};

#endif
