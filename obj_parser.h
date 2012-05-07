#ifndef __OBJ_PARSER_H
#define __OBJ_PARSER_H

#include "main.h"
#include "mesh.h"
#include <vector>

class CObjParser{
public:
	CObjParser(void);
	~CObjParser(void);
	
	void parse(const char *filename, CMesh *mesh);
};
#endif