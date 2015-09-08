#ifndef PV_VERTEX_H
#define PV_VERTEX_H

#include <glm/glm.hpp>

//NOTE: Our vertices don't have texture coordinates. We could probably think
//of a more general vertex class, but this is not really needed for our program.
struct Vertex{
    Vertex(void){}
    Vertex(const glm::vec3 coord, const glm::vec3 normal):
        _coord(coord), _normal(normal)
    {}
	glm::vec3 _coord;
	glm::vec3 _normal;
};

#endif
