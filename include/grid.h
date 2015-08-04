#ifndef PV_GRID_H
#define PV_GRID_H

#include <glm/glm.hpp>
#include <vector>
#include "coord_parser.h"

class IntersectionObject;
struct AABB{
    bool raycast(glm::vec3 o, glm::vec3 ray_dir, float &t)const;
	glm::vec3 bounds_[2];
};

//TODO: Need to think how we will handle clipping planes and hidden particles.
class Grid{
public:
    Grid(const SimConfig& config);
    ~Grid(void);
    bool raycast(glm::vec3 o, glm::vec3 dir, int& pid);
private:
    using Cell = std::vector<IntersectionObject*>;

    Cell* cells_;
    std::vector<IntersectionObject*> objects_;
    AABB scene_bounds_;
    glm::vec3 cell_size_;
    int n_cells_[3];
};

#endif