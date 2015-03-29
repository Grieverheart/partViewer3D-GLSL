#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include "include/coord_parser.h"

CCoordParser::CCoordParser(void){
}

CCoordParser::~CCoordParser(void){
}

void CCoordParser::parse(std::istream &in){
	
	std::string line;
	int line_count = 0;
	while(std::getline(in, line)){
		std::istringstream s(line);
		if(line_count == 0) s >> npart;
		else if(line_count == 1){
			for(int i = 0; i < 9; i++) s >> boxMatrix[i/3][i%3];
		}
		else{
			glm::vec3 v;
			glm::vec4 r;
			for(int i = 0; i < 3; i++) s >> v[i];
			for(int i = 0; i < 4; i++) s >> r[i];
            r.x = glm::radians(r.x);
			rotations.push_back(r);
			centers.push_back(v);
		}
		line_count++;
	}
}
