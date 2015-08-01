#ifndef PV_GRID_H
#define PV_GRID_H

#include <glm/glm.hpp>
#include "coord_parser.h"

//TODO: Need to think how we will handle clipping planes and hidden particles.
class Grid{
public:
    Grid(const SimConfig* config);
    bool raycast(glm::vec3 o, glm::vec3 dir, int& pid);
private:
    glm::vec3 scene_bounds_[2];
    glm::vec3 cell_size_;
    int n_cells_;
};

#endif
