#ifndef __OBJ_PARSER_H
#define __OBJ_PARSER_H

#include <string>

class CMesh;

void parse_obj(const char *filename, CMesh *mesh, std::string shading_model);

#endif
