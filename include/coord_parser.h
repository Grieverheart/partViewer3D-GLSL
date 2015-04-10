#ifndef __COORD_PARSER_H
#define __COORD_PARSER_H

#include <glm/glm.hpp>
#include "mesh.h"

//TODO: Destructor/Constructor!!
struct SimConfig{
    glm::vec3* pos;
    glm::vec4* rot;
    glm::mat3  box;
    int n_part;
    int n_meshes;
    int* mesh_id;
    CMesh* meshes;
};

SimConfig parse_config(const char* file_path);

#endif
