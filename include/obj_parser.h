#ifndef __OBJ_PARSER_H
#define __OBJ_PARSER_H

#include "main.h"
#include "mesh.h"
#include <vector>
#include <string>

class CObjParser{
public:
	CObjParser(void);
	~CObjParser(void);
	
	void parse(const char *filename, CMesh *mesh, std::string shading_model);
};
#endif