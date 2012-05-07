#ifndef __COORD_PARSER_H
#define __COORD_PARSER_H

#include "main.h"
#include <vector>

class CCoordParser{
public:
	CCoordParser(void);
	~CCoordParser(void);
	
	void parse(const char *filename);
	
	std::vector<glm::vec4> rotations;
	std::vector<glm::vec3> centers;
	glm::mat3 boxMatrix;
	int npart;
};
#endif