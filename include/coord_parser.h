#ifndef __COORD_PARSER_H
#define __COORD_PARSER_H

#include <vector>
#include <glm/glm.hpp>

class CCoordParser{
public:
	CCoordParser(void);
	~CCoordParser(void);
	
	void parse(std::istream &in);
	
	std::vector<glm::vec4> rotations;
	std::vector<glm::vec3> centers;
	glm::mat3 boxMatrix;
	int npart;
};
#endif
