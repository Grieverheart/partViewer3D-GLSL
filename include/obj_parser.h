#ifndef PV_OBJ_PARSER_H
#define PV_OBJ_PARSER_H

#include <vector>
#include <string>
#include "vertex.h"
#include "shape.h"

//TODO: Change shading model type
void parse_obj(const char *filename, Shape::Mesh& mesh, std::string shading_model);

#endif
